#include <csignal>

#include <spdlog/spdlog.h>

#include "reactor/reactor.h"
#include "reactor/tcp_server.h"
#include "test_common.h"

static int gSignal = 0;

static void HandleSignal(int signal) {
    gSignal = signal;
}

namespace {

class TcpEchoServer {
public:
    TcpEchoServer() = default;

    ~TcpEchoServer() = default;

public:
    int Initialize() {
        reactor_ = std::make_shared<network::Reactor>(0);
        auto ret = reactor_->Initialize();
        if (ret != 0) {
            SPDLOG_ERROR("reactor initialize failed");

            return ret;
        }

        tcp_server_ = std::make_shared<network::TcpServer>(reactor_);
        tcp_server_->SetConnectionCallback([this](const std::shared_ptr<network::TcpConnection> &conn) {
            HandleConnection(conn);
        });

        return 0;
    }

    int Start(uint16_t port, const std::string &host = "0.0.0.0") {
        auto ret = reactor_->Start();
        if (ret != 0) {
            SPDLOG_ERROR("reactor start failed");

            return ret;
        }

        ret = tcp_server_->Start(port, host);
        if (ret != 0) {
            SPDLOG_ERROR("server start failed");

            return ret;
        }

        return 0;
    }

private:
    void HandleConnection(const std::shared_ptr<network::TcpConnection> &conn) {
        conn->SetReadCallback([](network::TcpConnection *connection, const char *data, size_t length) {
            connection->Send(data, length);
        });

        conn->SetCloseCallback([this](network::TcpConnection *connection) {
            HandleDisconnected(connection);
        });

        connections_[conn->Id()] = conn;
    }

    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    void HandleDisconnected(network::TcpConnection *connection) { connections_.erase(connection->Id()); }

private:
    std::shared_ptr<network::Reactor> reactor_;
    std::shared_ptr<network::TcpServer> tcp_server_;
    std::unordered_map<uint64_t, std::shared_ptr<network::TcpConnection>> connections_;
};

} // namespace


int main() {
    SetupLogger(spdlog::level::trace);

    signal(SIGINT, HandleSignal);
    signal(SIGTRAP, HandleSignal);
    signal(SIGTERM, HandleSignal);

    auto server = std::make_shared<TcpEchoServer>();
    auto ret = server->Initialize();
    if (ret != 0) {
        SPDLOG_ERROR("server initialize failed");

        return ret;
    }

    constexpr uint16_t kPort = 8080;

    ret = server->Start(kPort);
    if (ret != 0) {
        SPDLOG_ERROR("server start failed");

        return ret;
    }

    while (gSignal == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    server.reset();

    SPDLOG_INFO("all finished");

    return 0;
}
