#include "h264_depacketizer.h"

#include <spdlog/spdlog.h>

namespace rtplib {

void H264Depacketizer::HandleRtpPayload(const std::shared_ptr<RtpPacket> *packets, size_t size) {
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
        auto nal_type = packet->payload[0] & 0x1F;
        if (nal_type == 24) {
            // STAP-A
            auto ret = HandleStapAPacket(packet, buffer + offset);
            bufferSize += ret;
            offset += ret;
        } else if (nal_type == 28) {
            // FU-A
            auto ret = HandleFuAPacket(packet, buffer + offset);
            bufferSize += ret;
            offset += ret;
        } else {
            // single nal
            auto ret = HandleSingleNal(packet, buffer + offset);
            bufferSize += ret;
            offset += ret;
        }
    }

    if (handler_ != nullptr) {
        handler_->HandleMediaData(buffer, bufferSize);
    }
}

size_t H264Depacketizer::HandleSingleNal(const std::shared_ptr<RtpPacket> &packet, uint8_t *buffer) {
    return CopyNalData(packet->payload, packet->payloadSize, buffer);
}

size_t H264Depacketizer::HandleStapAPacket(const std::shared_ptr<RtpPacket> &packet, uint8_t *buffer) {
    size_t dataLeft = packet->payloadSize;
    auto *data = packet->payload;
    size_t result = 0;

    // skip first byte
    ++data;
    --dataLeft;

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

size_t H264Depacketizer::HandleFuAPacket(const std::shared_ptr<RtpPacket> &packet, uint8_t *buffer) {
    // SPDLOG_TRACE("fu indicator 0x{:08x}, fu header 0x{:08x}", packet->payload[0], packet->payload[1]);

    size_t result = 0;
    auto s = packet->payload[1] & 0x80;
    // SPDLOG_DEBUG("s was {}", s);
    if (s > 0) {
        auto nal_head = static_cast<uint8_t>((packet->payload[0] & 0xE0) | (packet->payload[1] & 0x1F));
        memcpy(buffer, "\x00\x00\x00\x01", 4);
        buffer[4] = nal_head;
        result += 5;

        memcpy(buffer + result, packet->payload + 2, packet->payloadSize - 2);
        result += packet->payloadSize - 2;
    } else {
        memcpy(buffer + result, packet->payload + 2, packet->payloadSize - 2);
        result += packet->payloadSize - 2;
    }

    return result;
}

size_t H264Depacketizer::CopyNalData(const uint8_t *data, size_t len, uint8_t *dst) {
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
