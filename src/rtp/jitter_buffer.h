#ifndef RTPLIB_JITTER_BUFFER_H
#define RTPLIB_JITTER_BUFFER_H

#include <memory>
#include <vector>

#include "sequence_extender.h"
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
    void InsertRtpPacket(std::shared_ptr<RtpPacket> rtp_packet);

    void HandlRtpPackets();

    size_t GetContinuedPacketSize();

    bool ShouldDropCurrentFrame();

    void DropCurrentFrame();

    void OutputCurrentFrame(size_t size);

private:
    SequenceExtender sequence_extender_;
    std::shared_ptr<IPayloadDepacketizer> payload_depacketizer_;
    std::vector<std::shared_ptr<RtpPacket>> rtp_packets_;

    uint32_t last_consumed_timestamp_ = 0;
};

} // namespace rtplib

#endif // RTPLIB_JITTER_BUFFER_H
