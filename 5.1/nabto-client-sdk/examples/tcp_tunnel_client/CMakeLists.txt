set (CMAKE_CXX_STANDARD 14)

find_package( Threads )

set(src
  src/tcp_tunnel_client.cpp
  )

add_executable(tcp_tunnel_client "${src}")
target_link_libraries(tcp_tunnel_client cpp_wrapper client_examples_common 3rdparty_cxxopts 3rdparty_json ${CMAKE_THREAD_LIBS_INIT} )

install(TARGETS tcp_tunnel_client
  RUNTIME DESTINATION bin
  )
