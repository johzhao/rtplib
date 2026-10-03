#include "h265_depacketizer.h"

#include <cstring>

#include <spdlog/spdlog.h>

namespace rtplib {

void H265Depacketizer::HandleRtpPayload(const std::shared_ptr<RtpPacket> *packets, size_t size) {
    size_t bytes = 0;
    for (size_t i = 0; i < size; ++i) {
        bytes += packets[i]->payloadSize;
    }

    buffer_.resize(bytes * 2);

    auto *buffer = buffer_.data();
    size_t bufferSize = 0;
    size_t offset = 0;

    for (size_t i = 0; i < size; ++i) {
        auto packet = packets[i];
        auto nal_type = (packet->payload[0] >> 1) & 0x3F;
        if (nal_type == 48) {
            // Ap
            auto ret = HandleApPacket(packet, buffer + offset);
            bufferSize += ret;
            offset += ret;
        } else if (nal_type == 49) {
            // FU
            auto ret = HandleFuPacket(packet, buffer + offset);
            bufferSize += ret;
            offset += ret;
        } else {
            auto ret = HandleSingleNal(packet, buffer + offset);
            bufferSize += ret;
            offset += ret;
        }
    }

    if (handler_ != nullptr) {
        handler_->HandleMediaData(buffer, bufferSize);
    }
}

size_t H265Depacketizer::HandleSingleNal(const std::shared_ptr<RtpPacket> &packet, uint8_t *buffer) {
    return CopyNalData(packet->payload, packet->payloadSize, buffer);
}

size_t H265Depacketizer::HandleApPacket(const std::shared_ptr<RtpPacket> &packet, uint8_t *buffer) {
    size_t dataLeft = packet->payloadSize;
    auto *data = packet->payload;
    size_t result = 0;

    // skip first two byte
    data += 2;
    dataLeft += 2;

    while (dataLeft > 0) {
        const uint16_t naluSize = (static_cast<uint16_t>(data[0]) << 8) | static_cast<uint16_t>(data[1]);
        if (naluSize > dataLeft - 2) {
            SPDLOG_ERROR("nalu size was {}, data left was {}", naluSize, dataLeft);
            break;
        }

        data += 2;
        dataLeft -= 2;

        auto ret = CopyNalData(data, naluSize, buffer + result);
        result += ret;
        data += naluSize;
        dataLeft -= naluSize;
    }

    return result;
}

size_t H265Depacketizer::HandleFuPacket(const std::shared_ptr<RtpPacket> &packet, uint8_t *buffer) {
    // SPDLOG_DEBUG("fu indicator 0x{:08x} 0x{:08x}, fu header 0x{:08x}", packet->payload[0], packet->payload[1],
    // packet->payload[2]);

    constexpr int kPayloadOffset = 3;

    size_t result = 0;
    auto s = packet->payload[2] & 0x80;
    if (s > 0) {
        auto fuType = static_cast<uint8_t>(packet->payload[2] & 0x3F);
        uint8_t nalHeader0 = (packet->payload[0] & 0x81) | (fuType << 1);
        uint8_t nalHeader1 = packet->payload[1];

        memcpy(buffer, "\x00\x00\x00\x01", 4);
        buffer[4] = nalHeader0;
        buffer[5] = nalHeader1;
        result += 6;

        memcpy(buffer + result, packet->payload + kPayloadOffset, packet->payloadSize - kPayloadOffset);
        result += packet->payloadSize - kPayloadOffset;
    } else {
        memcpy(buffer + result, packet->payload + kPayloadOffset, packet->payloadSize - kPayloadOffset);
        result += packet->payloadSize - kPayloadOffset;
    }

    return result;
}

size_t H265Depacketizer::CopyNalData(const uint8_t *data, size_t len, uint8_t *dst) {
    if ((data[0] == '\x00' && data[1] == '\x00' && data[2] == '\x00' && data[3] == '\x01') ||
            (data[0] == '\x00' && data[1] == '\x00' && data[2] == '\x01')) {
        memcpy(dst, data, len);
        return len;
            }

    memcpy(dst, "\x00\x00\x00\x01", 4);
    memcpy(dst + 4, data, len);

    return len + 4;
}

} // namespace rtplib
