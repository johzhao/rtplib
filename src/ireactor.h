#ifndef RTPLIB_IREACTOR_H
#define RTPLIB_IREACTOR_H

#include <memory>

#include "ireactor_handler.h"

namespace rtp {

class IReactor {
public:
    virtual ~IReactor() = default;

    virtual int RegisterForListen(int fd, const std::shared_ptr<IReactorHandler> &handler) = 0;

    virtual int RegisterForDataRead(int fd, const std::shared_ptr<IReactorHandler> &handler) = 0;

    virtual int UnRegister(int fd) = 0;

    virtual int Start() = 0;

    virtual int Stop() = 0;
};

} // namespace rtp

#endif // RTPLIB_IREACTOR_H
