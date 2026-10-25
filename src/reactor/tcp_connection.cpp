#include "tcp_connection.h"

#include <sys/socket.h>
#include <unistd.h>

#include "spdlog/spdlog.h"

namespace network {

constexpr int kReceiveBufferSize = 64 * 1024;

TcpConnection::TcpConnection(std::shared_ptr<Reactor> reactor, std::shared_ptr<Channel> channel, std::string ip,
                             uint16_t port)
    : reactor_(std::move(reactor))
    , channel_(std::move(channel))
    , ip_(std::move(ip))
    , port_(port)
    , receive_buffer_(kReceiveBufferSize) {
}

TcpConnection::~TcpConnection() {
    Release();
}

int TcpConnection::Initialize() {
    channel_->SetReadCallback([this](Channel *channel) {
        OnReceived(channel);
    });

    channel_->SetWriteCallback([this](Channel *channel) {
        OnWritable(channel);
    });

    auto ret = reactor_->Add(channel_, EPOLLIN | EPOLLRDHUP | EPOLLET);
    if (ret != 0) {
        SPDLOG_ERROR("reactor add failed {}", ret);

        return ret;
    }

    id_ = channel_->Token();

    return 0;
}

uint64_t TcpConnection::Id() const {
    return id_;
}

void TcpConnection::SetReadCallback(ReadCallback callback) {
    read_callback_ = std::move(callback);
}

void TcpConnection::SetWriteCallback(WriteCompleteCallback callback) {
    write_callback_ = std::move(callback);
}

void TcpConnection::SetCloseCallback(CloseCallback callback) {
    close_callback_ = std::move(callback);
}

bool TcpConnection::IsConnected() const {
    return !closed_;
}

void TcpConnection::Send(const char *data, size_t length) {
    if (closed_) {
        return;
    }

    send_buffer_.resize(send_buffer_.size() + length);
    memcpy(send_buffer_.data() + send_buffer_offset_, data, length);

    SendImpl(channel_.get());
}

void TcpConnection::Shutdown() {
    if (closed_) {
        return;
    }

    closed_ = true;

    reactor_->Remove(channel_);
    channel_.reset();

    if (close_callback_ != nullptr) {
        close_callback_(this);
    }
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void TcpConnection::OnReceived(Channel *channel) {
    while (true) {
        auto ret = recv(channel->Fd(), receive_buffer_.data(), receive_buffer_.size(), 0);
        if (ret > 0) {
            HandleReceivedData(receive_buffer_.data(), ret);

            continue;
        }

        if (ret == 0) {
            HandleDisconnected();

            return;
        }

        if (errno == EINTR) {
            continue;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
    }
}

void TcpConnection::HandleReceivedData(const char *buffer, size_t len) {
    if (read_callback_ != nullptr) {
        read_callback_(this, buffer, len);
    }
}

void TcpConnection::HandleDisconnected() {
    closed_ = true;

    reactor_->Remove(channel_);
    channel_.reset();

    if (close_callback_ != nullptr) {
        close_callback_(this);
    }
}

void TcpConnection::OnWritable(Channel *channel) {
    if (closed_) {
        return;
    }

    SendImpl(channel);
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void TcpConnection::SendImpl(Channel *channel) {
    if (closed_) {
        send_buffer_.clear();
        send_buffer_offset_ = 0;

        return;
    }

    auto ret = send(channel->Fd(), send_buffer_.data() + send_buffer_offset_, send_buffer_.size() - send_buffer_offset_,
                    MSG_NOSIGNAL);
    if (ret > 0) {
        send_buffer_offset_ += ret;
        if (send_buffer_offset_ == send_buffer_.size()) {
            send_buffer_.clear();
            send_buffer_offset_ = 0;

            reactor_->Modify(channel->shared_from_this(), EPOLLIN | EPOLLRDHUP | EPOLLET);

            if (write_callback_ != nullptr) {
                write_callback_(this);
            }
        } else {
            reactor_->Modify(channel->shared_from_this(), EPOLLIN | EPOLLRDHUP | EPOLLET | EPOLLOUT);
        }
    }
}

void TcpConnection::Release() {
    if (!closed_) {
        reactor_->Remove(channel_);
    }
}

} // namespace network
