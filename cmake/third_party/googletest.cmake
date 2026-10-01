set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)
FetchContent_Declare(
        googletest
        SOURCE_DIR      ${CMAKE_SOURCE_DIR}/third_party/googletest
)
FetchContent_MakeAvailable(googletest)
include_directories(SYSTEM third_party/googletest/googletest/include)
