#ifndef RTPLIB_RTP_STREAM_MANAGER_H
#define RTPLIB_RTP_STREAM_MANAGER_H

#include <memory>
#include <mutex>
#include <unordered_map>

#include "imedia_handler.h"
#include "jitter_buffer.h"
#include "media_type.h"
#include "rtp_packet.h"

namespace rtplib {

class RtpStreamManager {
public:
    RtpStreamManager() = default;

    ~RtpStreamManager() = default;

public:
    void HandleRtpPacket(const std::shared_ptr<RtpPacket> &packet);

    void RegisterMediaHandler(uint32_t ssrc, MediaType media_type, const std::shared_ptr<IMediaHandler> &handler);

    void UnregisterMediaHandler(uint32_t ssrc);

private:
    std::shared_ptr<JitterBuffer> FindJitterBuffer(uint32_t ssrc);

    std::shared_ptr<IPayloadDepacketizer> CreatePayloadDepacketizer(MediaType media_type);

private:
    std::mutex mutex_;
    std::unordered_map<uint32_t, std::shared_ptr<JitterBuffer>> jitter_buffers_;
};

} // namespace rtplib

#endif // RTPLIB_RTP_STREAM_MANAGER_H
