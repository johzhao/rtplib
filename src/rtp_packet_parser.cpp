#include "rtp_packet_parser.h"

#include <spdlog/spdlog.h>

namespace rtplib {

std::shared_ptr<RtpPacket> RtpPacketParser::HandleRtpPacket(const uint8_t *data, size_t size) {
    if (data == nullptr || size < 12) {
        SPDLOG_ERROR("invalid data size {}", size);

        return nullptr;
    }

    auto packet = std::make_shared<RtpPacket>();

    // Byte 0
    packet->version = (data[0] >> 6) & 0x03;
    if (packet->version != 2) {
        SPDLOG_ERROR("invalid rtp version {}", packet->version);

        return nullptr;
    }

    packet->padding = (data[0] & 0x20) != 0;
    packet->extension = (data[0] & 0x10) != 0;
    packet->csrcCount = data[0] & 0x0F;

    // Byte 1
    packet->marker = (data[1] & 0x80) != 0;

    packet->payloadType = data[1] & 0x7F;

    // Sequence Number
    packet->sequence = (static_cast<uint16_t>(data[2]) << 8) | static_cast<uint16_t>(data[3]);

    // Timestamp
    packet->timestamp = (static_cast<uint32_t>(data[4]) << 24) | (static_cast<uint32_t>(data[5]) << 16) |
                        (static_cast<uint32_t>(data[6]) << 8) | static_cast<uint32_t>(data[7]);

    // SSRC
    packet->ssrc = (static_cast<uint32_t>(data[8]) << 24) | (static_cast<uint32_t>(data[9]) << 16) |
                   (static_cast<uint32_t>(data[10]) << 8) | static_cast<uint32_t>(data[11]);

    size_t offset = 12;

    // CSRC
    const size_t csrcBytes = static_cast<size_t>(packet->csrcCount) * 4;
    if (size < offset + csrcBytes) {
        SPDLOG_ERROR("incorrect rtp packet length, ignore it");

        return nullptr;
    }

    offset += csrcBytes;

    // RTP Header Extension
    if (packet->extension) {
        // Extension header:
        // 2 bytes profile
        // 2 bytes length (32-bit words)
        if (size < offset + 4) {
            SPDLOG_ERROR("incorrect rtp packet length, ignore it");

            return nullptr;
        }

        const uint16_t extensionLength =
                (static_cast<uint16_t>(data[offset + 2]) << 8) | static_cast<uint16_t>(data[offset + 3]);

        offset += 4;

        const size_t extensionBytes = static_cast<size_t>(extensionLength) * 4;
        if (size < offset + extensionBytes) {
            SPDLOG_ERROR("incorrect rtp packet length, ignore it");

            return nullptr;
        }

        offset += extensionBytes;
    }

    if (offset > size) {
        SPDLOG_ERROR("incorrect rtp packet length, ignore it");

        return nullptr;
    }

    packet->payloadSize = size - offset;
    packet->payload = new uint8_t[packet->payloadSize];
    memcpy(packet->payload, data + offset, packet->payloadSize);

    // Padding
    if (packet->padding) {
        if (packet->payloadSize == 0) {
            SPDLOG_ERROR("incorrect rtp packet length, ignore it");

            return nullptr;
        }

        const uint8_t paddingBytes = packet->payload[packet->payloadSize - 1];
        if (paddingBytes == 0 || paddingBytes > packet->payloadSize) {
            SPDLOG_ERROR("incorrect rtp packet length, ignore it");

            return nullptr;
        }

        packet->payloadSize -= paddingBytes;
    }

    return packet;
}

void RtpPacketParser::HandleRtcpPacket(const uint8_t *data, size_t size) {
    // TODO: not support for now
}

} // namespace rtplib
