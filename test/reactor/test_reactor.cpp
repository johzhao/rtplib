#include <csignal>
#include <iostream>
#include <unistd.h>

#include <arpa/inet.h>
#include <thread>

#include <spdlog/spdlog.h>

#include "ireactor_handler.h"
#include "reactor/reactor.h"
#include "test_common.h"

namespace {

constexpr int kReceiveBufferSize = 100 * 1024;

class EchoTcpServer {
public:
    explicit EchoTcpServer(const std::weak_ptr<network::Reactor> &reactor)
        : reactor_(reactor)
        , buffer_(kReceiveBufferSize) {}

    ~EchoTcpServer() { Release(); }

public:
    int Initialize(const std::string &host, uint16_t port) {
        listen_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
        if (listen_fd_ == -1) {
            SPDLOG_ERROR("create listen socket failed");
            Release();

            return -1;
        }

        int opt = 1;
        if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            SPDLOG_ERROR("setsockopt SO_REUSEADDR failed");
            Release();

            return -1;
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);
        server_addr.sin_port = htons(port);

        if (bind(listen_fd_, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
            SPDLOG_ERROR("bind failed");
            Release();

            return -1;
        }

        constexpr int kPendingListenConnections = 128;
        if (listen(listen_fd_, kPendingListenConnections) == -1) {
            SPDLOG_ERROR("listen failed");
            Release();

            return -1;
        }

        auto listen_channel = std::make_shared<network::Channel>(listen_fd_);
        listen_channel->SetReadCallback([this](const std::shared_ptr<network::Channel> &channel) {
            OnAccept();
        });

        auto strong_reactor = reactor_.lock();
        if (strong_reactor != nullptr) {
            strong_reactor->Add(listen_channel, EPOLLIN | EPOLLRDHUP | EPOLLET);
        }

        return 0;
    }

private:
    void OnAccept() {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = -1;
        while (true) {
            client_fd = accept4(listen_fd_, reinterpret_cast<sockaddr *>(&client_addr), &client_len,
                                SOCK_NONBLOCK | SOCK_CLOEXEC);
            if (client_fd >= 0) {
                HandleNewConnection(client_addr, client_fd);

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

    void HandleNewConnection(const sockaddr_in &address, int client_fd) {
        char client_ip[INET_ADDRSTRLEN]{};
        inet_ntop(AF_INET, &address.sin_addr, client_ip, sizeof(client_ip));

        SPDLOG_INFO("received connection from {}:{} as channel {}", client_ip, ntohs(address.sin_port), client_fd);

        auto client_channel = std::make_shared<network::Channel>(client_fd);
        client_channel->SetReadCallback([this](const std::shared_ptr<network::Channel> &channel) {
            OnRead(channel);
        });

        auto strong_reactor = reactor_.lock();
        if (strong_reactor != nullptr) {
            strong_reactor->Add(client_channel, EPOLLIN | EPOLLRDHUP | EPOLLET);
        }
    }

    void OnRead(const std::shared_ptr<network::Channel> &channel) {
        while (true) {
            auto ret = recv(channel->Fd(), buffer_.data(), buffer_.size(), 0);
            if (ret > 0) {
                HandleReceivedData(channel, buffer_.data(), ret);

                continue;
            }

            if (ret == 0) {
                HandleDisconnected(channel);

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

    void HandleReceivedData(const std::shared_ptr<network::Channel> &channel, const char *buffer, size_t len) {
        // SPDLOG_INFO("received {} bytes from channel {}", ret, channel->Fd());
        send(channel->Fd(), buffer, len, MSG_NOSIGNAL);
    }

    void HandleDisconnected(const std::shared_ptr<network::Channel> &channel) {
        SPDLOG_INFO("channel {} closed", channel->Fd());

        auto strong_reactor = reactor_.lock();
        if (strong_reactor != nullptr) {
            strong_reactor->Remove(channel);
        }
    }

    void Release() {
        SPDLOG_DEBUG("server release in");
        if (listen_fd_ >= 0) {
            close(listen_fd_);
            listen_fd_ = -1;
        }
        SPDLOG_DEBUG("server release out");
    }

private:
    std::weak_ptr<network::Reactor> reactor_;
    int listen_fd_ = -1;
    std::vector<char> buffer_;
};

} // namespace

static int gSignal = 0;

static void HandleSignal(int signal) {
    gSignal = signal;
}

int main() {
    SetupLogger(spdlog::level::debug);

    signal(SIGINT, HandleSignal);
    signal(SIGTRAP, HandleSignal);
    signal(SIGTERM, HandleSignal);

    auto reactor = std::make_shared<network::Reactor>(0);
    auto ret = reactor->Initialize();
    if (ret != 0) {
        SPDLOG_ERROR("reactor initialize failed");
        return -1;
    }

    reactor->Start();

    constexpr int kPort = 8080;

    auto server = std::make_shared<EchoTcpServer>(reactor);
    ret = server->Initialize("0.0.0.0", kPort);
    if (ret != 0) {
        SPDLOG_ERROR("failed to initialize reactor");

        return 1;
    }

    while (gSignal == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    reactor->Stop();

    server.reset();
    reactor.reset();

    SPDLOG_INFO("all finished");

    return 0;
}
