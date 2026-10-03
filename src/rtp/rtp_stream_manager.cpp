#include "rtp_stream_manager.h"

#include <sys/stat.h>

#include <spdlog/spdlog.h>

#include "payload_depacketizer/h264_depacketizer.h"
#include "payload_depacketizer/h265_depacketizer.h"

namespace rtplib {

void RtpStreamManager::HandleRtpPacket(std::shared_ptr<RtpPacket> packet) {
    const std::scoped_lock lock(mutex_);

    auto jitter_buffer = FindJitterBuffer(packet->ssrc);
    if (jitter_buffer == nullptr) {
        return;
    }

    jitter_buffer->PushRtpPacket(std::move(packet));
}

void RtpStreamManager::RegisterMediaHandler(uint32_t ssrc, MediaType media_type,
                                            const std::shared_ptr<IMediaHandler> &handler) {
    const std::scoped_lock lock(mutex_);

    auto jitter_buffer = FindJitterBuffer(ssrc);
    if (jitter_buffer != nullptr) {
        // TODO: support update handler later
        SPDLOG_ERROR("the ssrc {} already registered", ssrc);

        return;
    }

    // create the jitter buffer and bind the handler
    auto payload_depacketizer = CreatePayloadDepacketizer(media_type);
    if (payload_depacketizer == nullptr) {
        SPDLOG_ERROR("create payload depacketizer with media type {} failed", static_cast<int>(media_type));

        return;
    }

    payload_depacketizer->SetMediaHandler(handler);

    jitter_buffer = std::make_shared<JitterBuffer>();
    jitter_buffer->SetPayloadDepacketizer(payload_depacketizer);

    jitter_buffers_[ssrc] = jitter_buffer;
}

void RtpStreamManager::UnregisterMediaHandler(uint32_t ssrc) {
    const std::scoped_lock lock(mutex_);

    jitter_buffers_.erase(ssrc);
}

std::shared_ptr<JitterBuffer> RtpStreamManager::FindJitterBuffer(uint32_t ssrc) {
    if (auto it = jitter_buffers_.find(ssrc); it != jitter_buffers_.end()) {
        return it->second;
    }

    return nullptr;
}

std::shared_ptr<IPayloadDepacketizer> RtpStreamManager::CreatePayloadDepacketizer(MediaType media_type) {
    switch (media_type) {
        case MediaType::H264:
            return std::shared_ptr<H264Depacketizer>();
        case MediaType::H265:
            return std::shared_ptr<H265Depacketizer>();
        default:
            SPDLOG_ERROR("unsupported media type {}", static_cast<int>(media_type));
            return nullptr;
    }
}

} // namespace rtplib
