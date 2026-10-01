#ifndef RTPLIB_JITTER_BUFFER_H
#define RTPLIB_JITTER_BUFFER_H

#include <memory>
#include <vector>

#include "payload_depacketizer/ipayload_depacketizer.h"
#include "rtp_packet.h"

namespace rtplib {

class JitterBuffer {
public:
    JitterBuffer() = default;

    ~JitterBuffer() = default;

public:
    void SetPayloadDepacketizer(const std::shared_ptr<IPayloadDepacketizer> &payload_depacketizer);

    void PushRtpPacket(std::shared_ptr<RtpPacket> rtp_packet);

private:
    std::shared_ptr<IPayloadDepacketizer> payload_depacketizer_;
    std::vector<std::shared_ptr<RtpPacket>> rtp_packets_;
};

} // namespace rtplib

#endif // RTPLIB_JITTER_BUFFER_H
