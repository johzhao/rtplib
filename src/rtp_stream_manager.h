#ifndef RTPLIB_RTP_STREAM_MANAGER_H
#define RTPLIB_RTP_STREAM_MANAGER_H

#include <memory>

#include "rtp_packet.h"

namespace rtplib {

class RtpStreamManager {
public:
    RtpStreamManager();

    ~RtpStreamManager() = default;

public:
    void HandleRtpPacket(std::shared_ptr<RtpPacket> packet);

private:
};

} // namespace rtplib

#endif // RTPLIB_RTP_STREAM_MANAGER_H
