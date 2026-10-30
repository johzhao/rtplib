add_library(timer STATIC
        timer_manager.cpp
)
target_link_libraries(timer PUBLIC
        Threads::Threads
        spdlog::spdlog
)
