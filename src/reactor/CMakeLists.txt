add_library(reactor STATIC
        reactor.cpp
        channel.cpp
        tcp_server.cpp
        tcp_connection.cpp
        tcp_client.cpp
)
target_link_libraries(reactor PUBLIC
        Threads::Threads
        spdlog::spdlog
)
