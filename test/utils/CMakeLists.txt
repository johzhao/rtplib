add_executable(test_base64
        test_base64.cpp
)
target_link_libraries(test_base64 PUBLIC
        dl
        gtest
        gtest_main
        utils
        asan
)
add_test(test_base64 NAME test_base64 COMMAND test_base64)

add_executable(test_md5
        test_md5.cpp
)
target_link_libraries(test_md5 PUBLIC
        dl
        gtest
        gtest_main
        utils
        asan
)
add_test(test_md5 NAME test_md5 COMMAND test_md5)
