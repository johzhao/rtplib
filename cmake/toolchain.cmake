if (NOT DEFINED BUILD_TOOLCHAIN OR BUILD_TOOLCHAIN STREQUAL "")
    set(BUILD_TOOLCHAIN "default")
endif ()

message(STATUS "Build toolchain " ${BUILD_TOOLCHAIN})

include(cmake/toolchain/${BUILD_TOOLCHAIN}.cmake)
