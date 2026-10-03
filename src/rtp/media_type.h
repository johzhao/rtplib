#ifndef RTPLIB_MEDIA_TYPE_H
#define RTPLIB_MEDIA_TYPE_H

#include <cstdint>

namespace rtplib {

enum class MediaType : uint8_t {
    H264 = 0,
    H265,
};

} // namespace rtplib

#endif // RTPLIB_MEDIA_TYPE_H
