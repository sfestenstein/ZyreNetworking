cmake_minimum_required(VERSION 3.20)

project(ZyreLib CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CZMQ REQUIRED libczmq)
pkg_check_modules(ZYRE REQUIRED libzyre)
pkg_check_modules(ZMQ REQUIRED libzmq)

file(GLOB SOURCES     "*.cpp")
file(GLOB HEADERS     "*.h")

add_library(ZyreLib SHARED ${SOURCES} ${HEADERS})

target_include_directories(ZyreLib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    ${CZMQ_INCLUDE_DIRS}
    ${ZYRE_INCLUDE_DIRS}
    ${ZMQ_INCLUDE_DIRS}
)

target_link_libraries(ZyreLib PRIVATE
    ${CZMQ_LIBRARIES}
    ${ZYRE_LIBRARIES}
    ${ZMQ_LIBRARIES}
)

# Also propagate link directories if pkg-config provided them (less ideal than
# proper imported targets, but keeps behavior similar to previous global link dirs).
if(CZMQ_LIBRARY_DIRS)
    target_link_directories(ZyreLib PRIVATE ${CZMQ_LIBRARY_DIRS})
endif()
if(ZYRE_LIBRARY_DIRS)
    target_link_directories(ZyreLib PRIVATE ${ZYRE_LIBRARY_DIRS})
endif()
if(ZMQ_LIBRARY_DIRS)
    target_link_directories(ZyreLib PRIVATE ${ZMQ_LIBRARY_DIRS})
endif()
