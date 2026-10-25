add_executable(test_reactor
        test_reactor.cpp
)
target_link_libraries(test_reactor PUBLIC
        reactor
        asan
)
add_test(test_reactor NAME test_reactor COMMAND test_reactor)

add_executable(test_tcp_server
        test_tcp_server.cpp
)
target_link_libraries(test_tcp_server PUBLIC
        reactor
        asan
)
add_test(test_tcp_server NAME test_tcp_server COMMAND test_tcp_server)
