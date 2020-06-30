set(src coap_client.cpp)

add_executable(simple_coap_get "${src}")
target_link_libraries(simple_coap_get 3rdparty_cxxopts nabto_client)

install(TARGETS simple_coap_get
  RUNTIME DESTINATION bin
  )
