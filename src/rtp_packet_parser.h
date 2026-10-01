#ifndef RTPLIB_RTP_PACKET_PARSER_H
#define RTPLIB_RTP_PACKET_PARSER_H

#include <memory>

#include "rtp_stream_manager.h"
#include "utils/buffer.h"

namespace rtplib {

class RtpPacketParser {
public:
    RtpPacketParser();

    ~RtpPacketParser() = default;

public:
    void HandleRtpPacket(const char *data, uint32_t size);

    void HandleRtcpPacket(const char *data, uint32_t size);

private:
    std::shared_ptr<RtpStreamManager> rtp_stream_manager_;
    utils::Buffer buffer_;
};

} // namespace rtplib

#endif //RTPLIB_RTP_PACKET_PARSER_H
