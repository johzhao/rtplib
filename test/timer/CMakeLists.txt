add_executable(test_timer
        test_timer.cpp
)
target_link_libraries(test_timer PUBLIC
        timer
)
add_test(NAME test_timer COMMAND test_timer)
