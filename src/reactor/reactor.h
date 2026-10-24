#ifndef RTPLIB_POLL_THREAD_H
#define RTPLIB_POLL_THREAD_H

#include <sys/epoll.h>

#include <atomic>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "channel.h"

namespace network {

class Reactor {
public:
    explicit Reactor(int id);

    ~Reactor();

public:
    int Initialize();

    int Add(const std::shared_ptr<Channel> &channel, uint32_t events);

    int Modify(const std::shared_ptr<Channel> &channel, uint32_t events);

    int Remove(const std::shared_ptr<Channel> &channel);

    int Start();

    int Stop();

private:
    void RunLoop();

    void DoDispatch(const uint32_t &events, const uint64_t &token);

    uint64_t GetToken();

    void Wakeup() const;

    void HandleWakeup() const;

    std::shared_ptr<Channel> FindChannel(const uint64_t &token);

    void Release();

private:
    int id_;
    int epoll_fd_ = 0;
    int wakeup_fd_ = 0;
    epoll_event *events_ = nullptr;

    std::mutex mutex_;
    uint64_t next_token_;
    std::unordered_map<uint64_t, std::shared_ptr<Channel>> channel_map_;

    std::atomic<bool> running_ = false;
    std::thread work_thread_;
};

} // namespace network

#endif // RTPLIB_POLL_THREAD_H
