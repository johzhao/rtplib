#include "udp_connection.h"

#include <arpa/inet.h>

#include <spdlog/spdlog.h>

namespace network {

static constexpr int kReceiveBufferSize = 64 * 1024;

UdpConnection::UdpConnection(std::shared_ptr<Reactor> reactor)
    : reactor_(std::move(reactor))
    , receive_buffer_(kReceiveBufferSize) {
}

UdpConnection::~UdpConnection() = default;

void UdpConnection::SetReadCallback(ReadCallback callback) {
    read_callback_ = std::move(callback);
}

void UdpConnection::SetWriteCompleteCallback(WriteCompleteCallback callback) {
    write_complete_callback_ = std::move(callback);
}

int UdpConnection::Initialize() {
    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        SPDLOG_ERROR("create udp socket failed");

        return -1;
    }

    channel_ = std::make_shared<Channel>(fd);

    int opt = 1;
    if (setsockopt(channel_->Fd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        SPDLOG_ERROR("set socket reuse address failed");

        return -1;
    }

    auto ret = reactor_->Add(channel_, EPOLLIN | EPOLLRDHUP | EPOLLET);
    if (ret != 0) {
        SPDLOG_ERROR("register reactor event failed {}", ret);

        return ret;
    }

    channel_->SetReadCallback([this](Channel *channel) {
        OnDataReceived(channel);
    });

    return 0;
}

int UdpConnection::BindLocal(uint16_t port, const std::string &host) {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    inet_pton(AF_INET, host.c_str(), &address.sin_addr);
    address.sin_port = htons(port);

    if (bind(channel_->Fd(), reinterpret_cast<sockaddr *>(&address), sizeof(address)) == -1) {
        SPDLOG_ERROR("udp bind to {}:{} failed", host, port);

        return -1;
    }

    return 0;
}

int UdpConnection::SendTo(const sockaddr_in &address, const char *data, size_t length) {
    auto ret = sendto(channel_->Fd(), data, length, 0, reinterpret_cast<const sockaddr *>(&address), sizeof(address));
    if (ret < 0) {
        SPDLOG_ERROR("sendto failed {} {}", errno, strerror(errno));

        return static_cast<int>(ret);
    }

    return 0;
}

void UdpConnection::OnDataReceived(Channel *channel) {
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        auto ret = recvfrom(channel->Fd(), receive_buffer_.data(), receive_buffer_.size(), 0,
                            reinterpret_cast<sockaddr *>(&client_addr), &client_len);
        if (ret == 0) {
            return;
        }

        if (errno < 0) {
            if (errno == EINTR) {
                continue;
            }

            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            }

            return;
        }

        HandleReceivedData(client_addr, receive_buffer_.data(), ret);
    }
}

void UdpConnection::HandleReceivedData(const sockaddr_in &address, const char *data, size_t length) {
    if (read_callback_ != nullptr) {
        read_callback_(this, address, data, length);
    }
}

} // namespace network
