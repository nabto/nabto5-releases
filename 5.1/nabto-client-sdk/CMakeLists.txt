cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(nabto-client-sdk)

include_directories(include)

if ("x_${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "x_Linux")
  LINK_DIRECTORIES(linux)
  add_definitions("-Wall")
elseif ("x_${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "x_Darwin")
  LINK_DIRECTORIES(macos)
  add_definitions("-Wall")
endif()

add_subdirectory(3rdparty/json)
add_subdirectory(3rdparty/cxxopts)
add_subdirectory(cpp_wrapper)
add_subdirectory(examples/common)

#add_subdirectory(examples/heat_pump)
add_subdirectory(examples/tcptunnel)
add_subdirectory(examples/stream_echo)
