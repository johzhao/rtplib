#ifndef RTPLIB_IREACTOR_HANDLER_H
#define RTPLIB_IREACTOR_HANDLER_H

#include <netinet/in.h>

namespace rtp {

class IReactorHandler {
public:
    virtual ~IReactorHandler() = default;

    virtual void HandleReceivedData(const char *data, size_t len, sockaddr *address, size_t address_length) = 0;

    virtual void HandleWritable() = 0;

    virtual void HandleNetworkError(int code) = 0;

    virtual void HandleDisconnected() = 0;
};

} // namespace rtp

#endif // RTPLIB_IREACTOR_HANDLER_H
