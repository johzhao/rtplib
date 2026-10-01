#ifndef RTPLIB_IMEDIA_HANDLER_H
#define RTPLIB_IMEDIA_HANDLER_H

#include <cstdint>

namespace rtplib {

class IMediaHandler {
public:
    virtual ~IMediaHandler() = default;

    virtual void HandleMediaData(const uint8_t *data, size_t size) = 0;
};

} // namespace rtplib

#endif // RTPLIB_IMEDIA_HANDLER_H
