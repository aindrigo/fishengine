file(GLOB_RECURSE FISH_RUNTIME_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

set(FISH_RUNTIME_DEPS
    fish::engine
    fish::common
    nlohmann_json
)
add_executable(fish_runtime ${FISH_RUNTIME_SOURCES})
add_executable(fish::runtime ALIAS fish_runtime)

add_dependencies(fish_runtime ${FISH_RUNTIME_DEPS})
set_property(TARGET fish_runtime PROPERTY CXX_STANDARD 23)

target_include_directories(fish_runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(fish_runtime PUBLIC ${FISH_RUNTIME_DEPS})
