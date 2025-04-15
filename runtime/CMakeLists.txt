file(GLOB_RECURSE FISH_RUNTIME_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(fish_runtime ${FISH_RUNTIME_SOURCES})
add_executable(fish::runtime ALIAS fish_runtime)

add_dependencies(fish_runtime fish::engine fish::common)
set_property(TARGET fish_runtime PROPERTY CXX_STANDARD 23)

target_include_directories(fish_runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(fish_runtime PUBLIC fish::engine fish::common)