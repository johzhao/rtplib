#include "test_common.h"

#include "timer/timer_manager.h"

static void HandlerFirstTimerEvent(timer::TimerManager::TimerId timer_id) {
    SPDLOG_INFO("first timer with id {} fired", timer_id);
}

static void HandlerSecondTimerEvent(timer::TimerManager::TimerId timer_id) {
    SPDLOG_INFO("second timer with id {} fired", timer_id);
}

int main() {
    SetupLogger(spdlog::level::debug);

    auto timer_manager = std::make_unique<timer::TimerManager>();

    auto timer_id_1 = timer_manager->AddTimer(2000, HandlerFirstTimerEvent); // NOLINT(*-magic-numbers)
    SPDLOG_INFO("create first timer with id {}", timer_id_1);

    auto timer_id_2 = timer_manager->AddTimer(1000, HandlerSecondTimerEvent);  // NOLINT(*-magic-numbers)
    SPDLOG_INFO("create second timer with id {}", timer_id_2);

    std::this_thread::sleep_for(std::chrono::milliseconds(3000)); // NOLINT(*-magic-numbers)

    return 0;
}
