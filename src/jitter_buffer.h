#ifndef RTPLIB_JITTER_BUFFER_H
#define RTPLIB_JITTER_BUFFER_H

#include <memory>

#include "rtp_packet.h"

namespace rtplib {

class JitterBuffer {
public:
    JitterBuffer();

    ~JitterBuffer() = default;

public:
    void PushRtpPacket(std::shared_ptr<RtpPacket> rtp_packet);
};

} // namespace rtplib

#endif //RTPLIB_JITTER_BUFFER_H
