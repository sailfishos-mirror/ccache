register_dependency(Fmt BUNDLED 11.1.4)

add_library(dep_fmt STATIC fmt/format.cc)
target_include_directories(dep_fmt SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(dep_fmt PRIVATE standard_settings)
