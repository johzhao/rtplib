#include "channel.h"

#include <unistd.h>

#include <spdlog/spdlog.h>

namespace network {

Channel::Channel(int fd)
    : fd_(fd) {
    SPDLOG_TRACE("channel {} was created", fd_);
}

Channel::~Channel() {
    if (fd_ >= 0) {
        close(fd_);
    }
    SPDLOG_TRACE("channel {} was destroyed", fd_);
}

int Channel::Fd() const {
    return fd_;
}

void Channel::HandleReadEvent() {
    if (read_callback_ != nullptr) {
        read_callback_(shared_from_this());
    }
}

void Channel::HandleWriteEvent() {
    if (write_callback_ != nullptr) {
        write_callback_(shared_from_this());
    }
}

void Channel::HandleErrorEvent() {
    if (error_callback_ != nullptr) {
        error_callback_(shared_from_this());
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

void Channel::SetErrorCallback(Callback callback) {
    error_callback_ = std::move(callback);
}

} // namespace network
