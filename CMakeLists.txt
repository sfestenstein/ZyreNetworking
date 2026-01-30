cmake_minimum_required(VERSION 3.20)

project(ZyreNetworking CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use ccache if available
find_program(CCACHE_PROGRAM ccache QUIET)
if(CCACHE_PROGRAM)
    message(STATUS "ccache found: ${CCACHE_PROGRAM} — enabling as compiler launcher")
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
else()
    message(STATUS "ccache not found — continuing without compiler launcher")
endif()

## Prefer a system or package-provided spdlog if available; fall back to FetchContent
find_package(spdlog QUIET)
if (NOT spdlog_FOUND)
    # Fetch spdlog for fast and easy logging when not already available
    include(FetchContent)
    FetchContent_Declare(
            spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG        v1.15.1
    )
    FetchContent_MakeAvailable(spdlog)
endif()

add_compile_options(-fPIC -Wall -Wextra -Wconversion -Wshadow -Wold-style-cast -Wnon-virtual-dtor)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(CommonUtils)
add_subdirectory(proto)
add_subdirectory(Tests)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CZMQ REQUIRED libczmq)
pkg_check_modules(ZYRE REQUIRED libzyre)
pkg_check_modules(ZMQ REQUIRED libzmq)

include_directories(${CZMQ_INCLUDE_DIRS} ${ZYRE_INCLUDE_DIRS} ${ZMQ_INCLUDE_DIRS})
link_directories(${CZMQ_LIBRARY_DIRS} ${ZYRE_LIBRARY_DIRS} ${ZMQ_LIBRARY_DIRS})

add_subdirectory(ZyreLib)

add_executable(publisher src/publisher_main.cpp)
add_executable(subscriber src/subscriber_main.cpp)

target_link_libraries(publisher ZyreLib protoMessages)
target_link_libraries(subscriber ZyreLib protoMessages)
