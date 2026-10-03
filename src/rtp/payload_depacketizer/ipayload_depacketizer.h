#ifndef RTPLIB_IPAYLOAD_DEPACKETIZER_H
#define RTPLIB_IPAYLOAD_DEPACKETIZER_H

#include <cstdint>
#include <memory>

#include "rtp/rtp_packet.h"
#include "rtp/imedia_handler.h"

namespace rtplib {

class IPayloadDepacketizer {
public:
    virtual ~IPayloadDepacketizer() = default;

public:
    void SetMediaHandler(const std::shared_ptr<IMediaHandler> &handler) { handler_ = handler; }

    virtual void HandleRtpPayload(const std::shared_ptr<RtpPacket> *packets, size_t size) = 0;

protected:
    std::shared_ptr<IMediaHandler> handler_;
};

} // namespace rtplib

#endif // RTPLIB_IPAYLOAD_DEPACKETIZER_H
