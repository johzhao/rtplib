#ifndef RTPLIB_POLL_THREAD_H
#define RTPLIB_POLL_THREAD_H

#include <sys/epoll.h>

#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#include "ireactor.h"

namespace rtp {

class Reactor : public IReactor {
public:
    Reactor();

    ~Reactor() override;

public:
    int Initialize();

    int RegisterForListen(int fd, const std::shared_ptr<IReactorHandler> &handler) override;

    int RegisterForDataRead(int fd, const std::shared_ptr<IReactorHandler> &handler) override;

    int UnRegisterForDataRead(int fd) override;

    int Start() override;

    int Stop() override;

private:
    class Connection {
    public:
        bool is_listen = false;
        int fd = 0;
        uint64_t token = 0;
        std::shared_ptr<IReactorHandler> handler = nullptr;
    };

    void RunLoop();

    uint64_t GetToken();

    void Wakeup() const;

    void HandleWakeup() const;

    void HandleRead(const uint64_t &token);

    void HandleWrite(const uint64_t &token);

    void HandleClose(const uint64_t &token);

    void ProcessPendingClose();

    std::shared_ptr<Connection> FindConnection(const uint64_t &token);

    void Release();

private:
    int epoll_fd_ = 0;
    int wakeup_fd_ = 0;
    epoll_event *events_ = nullptr;

    std::mutex mutex_;
    uint64_t next_token_;
    std::unordered_map<uint64_t, std::shared_ptr<Connection>> connections_;

    std::atomic<bool> running_ = false;
    std::thread work_thread_;

    std::vector<uint64_t> pending_close_;
};

} // namespace rtp

#endif // RTPLIB_POLL_THREAD_H
