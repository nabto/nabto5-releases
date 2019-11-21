set(src
  json_config.cpp
  timestamp.cpp
  )

add_library(client_examples_common "${src}")
target_link_libraries(client_examples_common 3rdparty_json 3rdparty_cxxopts)
target_include_directories(client_examples_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
