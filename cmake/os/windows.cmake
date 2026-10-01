cmake_policy(SET CMP0091 NEW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4100")   # 未引用的形参
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4820")   # 字节填充
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4668")   # 没有将定义为预处理器宏
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4514")   # 未引用的内联函数已移除
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4566")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4996")

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

if (MSVC)
    add_compile_options(/utf-8)
endif()
