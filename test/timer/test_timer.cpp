#include "test_common.h"

#include "timer/timer_manager.h"

static void HandlerTimerEvent(timer::TimerManager::TimerId timer_id) {
    SPDLOG_INFO("timer with id {} fired", timer_id);
}

int main() {
    SetupLogger(spdlog::level::debug);

    auto timer_manager = std::make_unique<timer::TimerManager>();

    auto timer_id_1 = timer_manager->AddTimer(1000, HandlerTimerEvent);
    SPDLOG_INFO("create a timer with id {}", timer_id_1);

    std::this_thread::sleep_for(std::chrono::milliseconds(1200));

    return 0;
}
