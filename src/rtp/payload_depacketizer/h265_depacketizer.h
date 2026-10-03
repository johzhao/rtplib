#ifndef RTPLIB_H265_DEPACKETIZER_H
#define RTPLIB_H265_DEPACKETIZER_H

#include <vector>

#include "ipayload_depacketizer.h"

namespace rtplib {

class H265Depacketizer : public IPayloadDepacketizer {
public:
    H265Depacketizer() = default;

    ~H265Depacketizer() override = default;

public:
    void HandleRtpPayload(const std::shared_ptr<RtpPacket> *packets, size_t size) override;

private:
    size_t HandleSingleNal(const std::shared_ptr<RtpPacket> &packet, uint8_t *buffer);

    size_t HandleApPacket(const std::shared_ptr<RtpPacket> &packet, uint8_t *buffer);

    size_t HandleFuPacket(const std::shared_ptr<RtpPacket> &packet, uint8_t *buffer);

    size_t CopyNalData(const uint8_t *data, size_t len, uint8_t *dst);

private:
    std::vector<uint8_t> buffer_;
};

} // namespace rtplib

#endif //RTPLIB_H265_DEPACKETIZER_H
