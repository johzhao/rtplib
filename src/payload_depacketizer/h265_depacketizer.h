#ifndef RTPLIB_H265_DEPACKETIZER_H
#define RTPLIB_H265_DEPACKETIZER_H

#include "ipayload_depacketizer.h"

namespace rtplib {

class H265Depacketizer : public IPayloadDepacketizer {
public:
    H265Depacketizer();

    ~H265Depacketizer() override;

public:
    void HandleRtpPayload(const std::shared_ptr<RtpPacket> *packets, size_t size) override;
};

} // namespace rtplib

#endif //RTPLIB_H265_DEPACKETIZER_H
