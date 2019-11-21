set (CMAKE_CXX_STANDARD 14)

find_package( Threads )

set(src
  src/stream_echo_client.cpp
  )

add_executable(stream_echo_client "${src}")
target_link_libraries(stream_echo_client cpp_wrapper client_examples_common 3rdparty_cxxopts 3rdparty_json ${CMAKE_THREAD_LIBS_INIT})
