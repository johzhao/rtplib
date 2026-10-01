#include "jitter_buffer.h"

namespace rtplib {

void JitterBuffer::SetPayloadDepacketizer(const std::shared_ptr<IPayloadDepacketizer> &payload_depacketizer) {
    payload_depacketizer_ = payload_depacketizer;
}

void JitterBuffer::PushRtpPacket(std::shared_ptr<RtpPacket> rtp_packet) {
    rtp_packets_.push_back(std::move(rtp_packet));

    // TODO: check if the cached packet was continued and finished

    if (rtp_packet->marker) {
        payload_depacketizer_->HandleRtpPayload(rtp_packets_.data(), rtp_packets_.size());
    }
}

} // namespace rtplib
