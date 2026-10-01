#ifndef RTPLIB_IPAYLOAD_DEPACKETIZER_H
#define RTPLIB_IPAYLOAD_DEPACKETIZER_H

#include <cstdint>

namespace rtplib {

class IPayloadDepacketizer {
public:
    virtual ~IPayloadDepacketizer() = default;

    virtual void HandleRtpPayload(const uint8_t *data, uint16_t size) = 0;
};

} // namespace rtplib

#endif //RTPLIB_IPAYLOAD_DEPACKETIZER_H
