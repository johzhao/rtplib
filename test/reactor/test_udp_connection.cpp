#include <csignal>


#include "test_common.h"

#include "reactor/udp_connection.h"

namespace {

class EchoUdpServer {
public:
    explicit EchoUdpServer(std::shared_ptr<network::Reactor> reactor)
        : reactor_(std::move(reactor))
        , connection_(std::make_shared<network::UdpConnection>(reactor_)) {}

public:
    int Bind(uint16_t port, const std::string &host = "0.0.0.0") {
        auto ret = connection_->Initialize();
        if (ret != 0) {
            return ret;
        }

        ret = connection_->BindLocal(port, host);
        if (ret != 0) {
            return ret;
        }

        connection_->SetReadCallback([this](network::UdpConnection *connection, const sockaddr_in &address,
                                            const char *data, size_t length) {
            OnReceivedData(address, data, length);
        });

        return 0;
    }

private:
    void OnReceivedData(const sockaddr_in &address, const char *data, size_t length) {
        connection_->SendTo(address, data, length);
    }

private:
    std::shared_ptr<network::Reactor> reactor_;
    std::shared_ptr<network::UdpConnection> connection_;
};

} // namespace

static std::atomic gRunningFlag = true;

static void HandleSignal(int signal) {
    gRunningFlag.store(false);
}

int main() {
    signal(SIGINT, HandleSignal);
    signal(SIGTERM, HandleSignal);

    SetupLogger(spdlog::level::debug);

    auto reactor = std::make_shared<network::Reactor>(0);
    auto ret = reactor->Initialize();
    if (ret != 0) {
        return 1;
    }
    reactor->Start();

    auto server = std::make_shared<EchoUdpServer>(reactor);
    ret = server->Bind(8080);
    if (ret != 0) {
        return 1;
    }

    while (gRunningFlag.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
