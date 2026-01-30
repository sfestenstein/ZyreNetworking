cmake_minimum_required(VERSION 3.21)
project(protoMessages LANGUAGES CXX)

# Find Protobuf
find_package(Protobuf REQUIRED)
if(NOT Protobuf_FOUND)
  message(FATAL_ERROR "Protobuf is required to build protocol buffers")
endif()
message(STATUS "Found Protobuf ${Protobuf_VERSION}")

# Gather all .proto files in this directory
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
if(NOT PROTO_FILES)
  message(WARNING "No .proto files found in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# Generate C++ sources from .proto files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# Create a shared library from the generated sources
add_library(${PROJECT_NAME} SHARED ${PROTO_SRCS} ${PROTO_HDRS} ${PROTO_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  VERSION 1.0.0
  SOVERSION 1
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Protobuf_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}  PUBLIC ${Protobuf_LIBRARIES})
