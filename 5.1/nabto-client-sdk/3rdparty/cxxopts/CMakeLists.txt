set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})


include_directories(${src_dir})

add_library(3rdparty_cxxopts INTERFACE)
target_include_directories(3rdparty_cxxopts INTERFACE ${src_dir})
