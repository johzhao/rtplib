#ifndef RTPLIB_H264_DEPACKETIZER_H
#define RTPLIB_H264_DEPACKETIZER_H

#include "ipayload_depacketizer.h"

namespace rtplib {

class H264Depacketizer : public IPayloadDepacketizer {
public:
    H264Depacketizer();

    ~H264Depacketizer() override = default;

public:
    void HandleRtpPayload(const uint8_t *data, uint16_t size) override;
};

} // namespace rtplib

#endif //RTPLIB_H264_DEPACKETIZER_H
