cmake_minimum_required(VERSION 3.10)
project(fedpubsub CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CZMQ REQUIRED libczmq)
pkg_check_modules(ZYRE REQUIRED libzyre)
pkg_check_modules(ZMQ REQUIRED libzmq)

include_directories(${CZMQ_INCLUDE_DIRS} ${ZYRE_INCLUDE_DIRS} ${ZMQ_INCLUDE_DIRS})
link_directories(${CZMQ_LIBRARY_DIRS} ${ZYRE_LIBRARY_DIRS} ${ZMQ_LIBRARY_DIRS})

add_library(fedlib STATIC src/zyre_node.cpp src/zyre_node.h)

add_executable(publisher src/publisher_main.cpp)
add_executable(subscriber src/subscriber_main.cpp)

target_link_libraries(fedlib PUBLIC ${CZMQ_LIBRARIES} ${ZYRE_LIBRARIES} ${ZMQ_LIBRARIES})
target_link_libraries(publisher fedlib ${CZMQ_LIBRARIES} ${ZYRE_LIBRARIES} ${ZMQ_LIBRARIES})
target_link_libraries(subscriber fedlib ${CZMQ_LIBRARIES} ${ZYRE_LIBRARIES} ${ZMQ_LIBRARIES})
