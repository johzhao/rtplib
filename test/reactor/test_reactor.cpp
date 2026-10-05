#include <csignal>
#include <unistd.h>

#include <arpa/inet.h>
#include <thread>

#include <spdlog/spdlog.h>

#include "ireactor.h"
#include "ireactor_handler.h"
#include "reactor/reactor.h"
#include "test_common.h"

namespace {

class EchoTcpServer : public rtp::IReactorHandler, public std::enable_shared_from_this<rtp::IReactorHandler> {
public:
    explicit EchoTcpServer(const std::shared_ptr<rtp::IReactor> &reactor)
        : reactor_(reactor) {}

    ~EchoTcpServer() override {
        Release();
    }

public:
    int Initialize(const std::string &host, uint16_t port) {
        listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd_ == -1) {
            SPDLOG_ERROR("create listen socket failed");
            Release();

            return -1;
        }

        // 允许快速重启服务端时复用端口
        int opt = 1;
        if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            SPDLOG_ERROR("setsockopt SO_REUSEADDR failed");
            Release();

            return -1;
        }

        // 2. 设置服务器地址
        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(port);

        // 3. bind
        if (bind(listen_fd_, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
            SPDLOG_ERROR("bind failed");
            Release();

            return -1;
        }

        // 4. listen
        constexpr int kPendingListenConnections = 128;
        if (listen(listen_fd_, kPendingListenConnections) == -1) {
            SPDLOG_ERROR("listen failed");
            Release();

            return -1;
        }

        reactor_->RegisterForListen(listen_fd_, shared_from_this());

        return 0;
    }

    void HandleReceivedData(int fd, const char *data, size_t len, sockaddr *address, size_t address_length) override {
        if (fd == listen_fd_) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);

            int client_fd = accept(listen_fd_, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
            if (client_fd == -1) {
                SPDLOG_ERROR("accept failed");

                return;
            }

            char client_ip[INET_ADDRSTRLEN]{};
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

            // std::cout << "Client connected: " << client_ip << ":" << ntohs(client_addr.sin_port) << '\n';

            reactor_->RegisterForDataRead(client_fd, shared_from_this());

            return;
        }

        send(fd, data, len, 0);
    }

    void HandleWritable(int fd) override {}

    void HandleNetworkError(int fd, int code) override {}

    void HandleDisconnected(int fd) override {
        SPDLOG_INFO("fd {} was disconnected", fd);
    }

private:
    void Release() {
        SPDLOG_DEBUG("server release in");
        if (listen_fd_ >= 0) {
            close(listen_fd_);
            listen_fd_ = -1;
        }
        SPDLOG_DEBUG("server release out");
    }

private:
    std::shared_ptr<rtp::IReactor> reactor_;
    int listen_fd_ = -1;
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

    auto reactor = std::make_shared<rtp::Reactor>();
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

    SPDLOG_INFO("all finished");

    return 0;
}
