add_executable(test_reactor
        test_reactor.cpp
)
target_link_libraries(test_reactor PUBLIC
        reactor
        asan
)
add_test(test_reactor NAME test_reactor COMMAND test_reactor)
