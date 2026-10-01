cmake_minimum_required(VERSION 4.0)

project(RtpLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

include(cmake/os.cmake)
include(cmake/toolchain.cmake)
include(cmake/third_party.cmake)
include(cmake/version.cmake)

include_directories(src)

add_library(asan INTERFACE)
target_compile_options(asan INTERFACE
        $<$<CONFIG:Debug>:
        -O0
        -g
        -fsanitize=address
        -fno-omit-frame-pointer
        >
)
target_link_options(asan INTERFACE
        $<$<CONFIG:Debug>:
        -fsanitize=address
        >
)

add_subdirectory(src)

option(BUILD_RTPLIB_CMAKE_TEST "Whether or not to build the tests" ON)
if (${BUILD_RTPLIB_CMAKE_TEST})
    message(STATUS "enable testing")
    enable_testing()
    include_directories(test)
    add_subdirectory(test)
endif ()
