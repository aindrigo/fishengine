add_library(fish_common INTERFACE )
add_library(fish::common ALIAS fish_common)

set_property(TARGET fish_common PROPERTY CXX_STANDARD 23)

target_include_directories(fish_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)