#include <csignal>

#include "test_common.h"

#include "reactor/reactor.h"
#include "reactor/tcp_client.h"

static bool gRunning = true;

static void HandleSingle(int signal) {
    gRunning = false;
}

int main() {
    signal(SIGINT, HandleSingle);
    signal(SIGTERM, HandleSingle);

    SetupLogger(spdlog::level::debug);

    auto reactor = std::make_shared<network::Reactor>(0);
    auto ret = reactor->Initialize();
    if (ret != 0) {
        SPDLOG_ERROR("reactor initialize failed {}", ret);

        return 1;
    }

    reactor->Start();

    auto client = std::make_shared<network::TcpClient>(reactor);
    ret = client->Initialize();
    if (ret != 0) {
        SPDLOG_ERROR("tcp client initialize failed {}", ret);

        return 1;
    }

    client->SetReadCallback([](network::TcpClient *c, const char *data, size_t length) {
        SPDLOG_INFO("tcp client received '{}'", data);
        c->Send(data, length);
    });

    client->SetConnectCallback([](network::TcpClient *c) {
        SPDLOG_INFO("client connected");
        const auto *data = "hello";
        c->Send(data, strlen(data));
    });

    client->SetCloseCallback([](network::TcpClient *c) {
        SPDLOG_INFO("client closed");
        gRunning = false;
    });

    const auto *kHost = "192.168.0.100";
    constexpr uint16_t kPort = 8888;
    ret = client->ConnectTo(kHost, kPort);
    if (ret != 0) {
        SPDLOG_ERROR("tcp client connect to {}:{} failed {}", kHost, kPort, ret);

        return 1;
    }

    while (gRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
