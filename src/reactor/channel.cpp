#include "channel.h"

#include <sys/epoll.h>

#include <spdlog/spdlog.h>

namespace network {

Channel::Channel(int fd)
    : fd_(fd) {
    SPDLOG_TRACE("channel {} created", fd_);
}

Channel::~Channel() {
    SPDLOG_TRACE("channel {} destroyed", fd_);
}

int Channel::Fd() const {
    return fd_;
}

void Channel::HandleEvents(uint32_t events) {
    if ((events & EPOLLIN) != 0) {
        if (read_callback_ != nullptr) {
            read_callback_(shared_from_this());
        }
    }

    if ((events & EPOLLOUT) != 0) {
        if (write_callback_ != nullptr) {
            write_callback_(shared_from_this());
        }
    }
}

uint64_t Channel::Token() const {
    return token_;
}

void Channel::SetToken(const uint64_t &token) {
    token_ = token;
}

void Channel::SetReadCallback(Callback callback) {
    read_callback_ = std::move(callback);
}

void Channel::SetWriteCallback(Callback callback) {
    write_callback_ = std::move(callback);
}

} // namespace network
