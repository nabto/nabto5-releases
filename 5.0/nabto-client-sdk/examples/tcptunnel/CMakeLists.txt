set (CMAKE_CXX_STANDARD 14)

find_package( Threads )

set(src
  src/tunnel_client.cpp
  )

add_executable(tcptunnel_client "${src}")
target_link_libraries(tcptunnel_client cpp_wrapper client_examples_common 3rdparty_cxxopts 3rdparty_json ${CMAKE_THREAD_LIBS_INIT} )
