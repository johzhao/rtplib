#ifndef RTPLIB_RTP_PACKET_PARSER_H
#define RTPLIB_RTP_PACKET_PARSER_H

#include <memory>

#include "rtp_stream_manager.h"

namespace rtplib {

class RtpPacketParser {
public:
    RtpPacketParser() = default;

    ~RtpPacketParser() = default;

public:
    std::shared_ptr<RtpPacket> HandleRtpPacket(const uint8_t *data, size_t size);

    void HandleRtcpPacket(const uint8_t *data, size_t size);

private:
    std::shared_ptr<RtpStreamManager> rtp_stream_manager_;
};

} // namespace rtplib

#endif //RTPLIB_RTP_PACKET_PARSER_H
