#include "jitter_buffer.h"

#include <sys/stat.h>

#include <spdlog/spdlog.h>

namespace rtplib {

static constexpr int kMaxWaitInMs = 20;

void JitterBuffer::SetPayloadDepacketizer(const std::shared_ptr<IPayloadDepacketizer> &payload_depacketizer) {
    payload_depacketizer_ = payload_depacketizer;
}

void JitterBuffer::PushRtpPacket(std::shared_ptr<RtpPacket> rtp_packet) {
    rtp_packet->extended_sequence = sequence_extender_.Extend(rtp_packet->sequence);

    // is a late packet?
    if (rtp_packet->timestamp < last_consumed_timestamp_) {
        SPDLOG_DEBUG("drop the late frame with ssrc {} and sequence {}", rtp_packet->ssrc, rtp_packet->sequence);

        return;
    }

    InsertRtpPacket(std::move(rtp_packet));

    HandlRtpPackets();
}

void JitterBuffer::InsertRtpPacket(std::shared_ptr<RtpPacket> rtp_packet) {
    auto count = rtp_packets_.size();
    for (size_t i = 0; i < count; ++i) {
        auto &data = rtp_packets_[i];
        if (data->extended_sequence < rtp_packet->extended_sequence) {
            continue;
        }

        if (data->extended_sequence == rtp_packet->extended_sequence) {
            SPDLOG_TRACE("the rtp packet with ssrc {} and sequence {} was duplicated, replace old one", data->ssrc,
                         data->sequence);
            rtp_packets_[i] = std::move(rtp_packet);

            return;
        }

        if (data->extended_sequence > rtp_packet->extended_sequence) {
            SPDLOG_TRACE("received lost rtp packet with ssrc {} and sequence {}", data->ssrc, data->sequence);
            rtp_packets_.insert(rtp_packets_.begin() + static_cast<uint16_t>(i), std::move(rtp_packet));

            return;
        }
    }

    rtp_packets_.push_back(std::move(rtp_packet));
}

void JitterBuffer::HandlRtpPackets() {
    while (true) {
        auto count = GetContinuedPacketSize();
        if (count > 0) {
            OutputCurrentFrame(count);

            continue;
        }

        if (ShouldDropCurrentFrame()) {
            DropCurrentFrame();

            continue;
        }

        break;
    }
}

size_t JitterBuffer::GetContinuedPacketSize() {
    if (rtp_packets_.empty()) {
        return 0;
    }

    if (rtp_packets_[0]->marker) {
        return 1;
    }

    auto start_seq = rtp_packets_[0]->extended_sequence;
    auto count = rtp_packets_.size();
    for (size_t i = 1; i < count; ++i) {
        const auto &data = rtp_packets_[i];
        if (data->extended_sequence != ++start_seq) {
            return 0;
        }

        if (data->marker) {
            return i + 1;
        }
    }

    return 0;
}

bool JitterBuffer::ShouldDropCurrentFrame() {
    if (rtp_packets_.empty()) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    if (timestamp - rtp_packets_[0]->received_timestamp > kMaxWaitInMs) {
        SPDLOG_TRACE("current frame received at {}, which is {} milliseconds ago, should drop it", rtp_packets_[0]->received_timestamp,
                     timestamp - rtp_packets_[0]->received_timestamp);

        return true;
    }

    return false;
}

void JitterBuffer::DropCurrentFrame() {
    if (rtp_packets_.empty()) {
        return;
    }

    uint16_t drop_count = 0;
    auto current_timestamp = rtp_packets_.front()->timestamp;
    auto count = rtp_packets_.size();
    for (size_t i = 0; i < count; ++i) {
        const auto &data = rtp_packets_[i];
        if (data->timestamp == current_timestamp) {
            ++drop_count;
            continue;
        }

        break;
    }

    if (drop_count > 0) {
        SPDLOG_TRACE("drop current frame with {} packets", drop_count);
        last_consumed_timestamp_ = rtp_packets_[drop_count - 1]->timestamp;
        rtp_packets_.erase(rtp_packets_.begin(), rtp_packets_.begin() + drop_count);
    }
}

void JitterBuffer::OutputCurrentFrame(size_t size) {
    SPDLOG_TRACE("output {} packets as one frame", size);
    payload_depacketizer_->HandleRtpPayload(rtp_packets_.data(), size);

    sequence_extender_.Reset();
    last_consumed_timestamp_ = rtp_packets_[size]->timestamp;

    rtp_packets_.erase(rtp_packets_.begin(), rtp_packets_.begin() + static_cast<uint16_t>(size));
    for (auto &it: rtp_packets_) {
        it->extended_sequence = sequence_extender_.Extend(it->sequence);
    }
}

} // namespace rtplib
