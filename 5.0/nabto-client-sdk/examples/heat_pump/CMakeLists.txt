set(src
  src/main.cpp
  )

add_executable(heat_pump_client ${src})

target_link_libraries(heat_pump_client 3rdparty_cxxopts cpp_wrapper 3rdparty_json client_examples_common)
