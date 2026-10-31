#include "tcp_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <spdlog/spdlog.h>

namespace network {

TcpServer::TcpServer(std::shared_ptr<Reactor> reactor)
    : reactor_(std::move(reactor)) {
}

TcpServer::~TcpServer() {
    Release();
}

void TcpServer::SetConnectionCallback(ConnectionCallback callback) {
    connection_callback_ = std::move(callback);
}

int TcpServer::Start(uint16_t port, const std::string &host) {
    listen_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (listen_fd_ == -1) {
        SPDLOG_ERROR("create listen socket failed");
        Release();

        return -1;
    }

    int opt = 1;
    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        SPDLOG_ERROR("set socket reuse address failed");
        Release();

        return -1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    inet_pton(AF_INET, host.c_str(), &address.sin_addr);
    address.sin_port = htons(port);

    if (bind(listen_fd_, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == -1) {
        SPDLOG_ERROR("bind failed");
        Release();

        return -1;
    }

    constexpr int kPendingListenConnections = 512;
    if (listen(listen_fd_, kPendingListenConnections) == -1) {
        SPDLOG_ERROR("listen failed");
        Release();

        return -1;
    }

    auto listen_channel = std::make_shared<Channel>(listen_fd_);
    // ReSharper disable once CppParameterNeverUsed
    listen_channel->SetReadCallback([this](Channel *channel) {
        OnAccept();
    });

    reactor_->Add(listen_channel, EPOLLIN | EPOLLRDHUP | EPOLLET);

    return 0;
}

void TcpServer::OnAccept() {
    sockaddr_in address{};
    socklen_t client_len = sizeof(address);
    int client_fd = -1;
    while (true) {
        client_fd = accept4(listen_fd_, reinterpret_cast<sockaddr *>(&address), &client_len,
                            SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (client_fd >= 0) {
            HandleNewConnection(address, client_fd);

            continue;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }

        if (errno == EINTR) {
            continue;
        }

        break;
    }
}

void TcpServer::HandleNewConnection(const sockaddr_in &address, int client_fd) {
    char client_ip[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &address.sin_addr, client_ip, sizeof(client_ip));
    uint16_t client_port = ntohs(address.sin_port);

    SPDLOG_DEBUG("received connection from {}:{} as channel {}", client_ip, client_port, client_fd);

    auto client_channel = std::make_shared<Channel>(client_fd);

    if (connection_callback_ != nullptr) {
        auto connection = std::make_shared<TcpConnection>(reactor_);
        auto ret = connection->Initialize(client_channel, client_ip, client_port);
        if (ret != 0) {
            SPDLOG_ERROR("initialize connection failed");

            return;
        }

        connection_callback_(connection);
    }
}

void TcpServer::Release() {
    if (listen_fd_ >= 0) {
        close(listen_fd_);
        listen_fd_ = -1;
    }
}

} // namespace network
