add_library(reactor STATIC
        reactor.cpp
        channel.cpp
)
target_link_libraries(reactor PUBLIC
        Threads::Threads
        spdlog::spdlog
)
