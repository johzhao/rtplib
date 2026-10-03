#ifndef RTPLIB_RTP_PACKET_H
#define RTPLIB_RTP_PACKET_H

#include <cstdint>

namespace rtplib {

class RtpPacket {
public:
    uint8_t version = 0;
    bool padding = false;
    bool extension = false;

    uint8_t csrcCount = 0;
    bool marker = false;
    uint8_t payloadType = 0;

    uint16_t sequence = 0;
    uint32_t timestamp = 0;
    uint32_t ssrc = 0;

    uint16_t payloadSize = 0;
    uint8_t *payload = nullptr;

    uint32_t extended_sequence = 0;
    uint64_t received_timestamp = 0;
};

} // namespace rtplib

#endif // RTPLIB_RTP_PACKET_H
