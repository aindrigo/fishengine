file(GLOB_RECURSE FISH_ENGINE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(fish_engine STATIC ${FISH_ENGINE_SOURCES})
add_library(fish::engine ALIAS fish_engine)

add_dependencies(fish_engine fish::common glfw glad EnTT::EnTT glm::glm)
set_property(TARGET fish_engine PROPERTY CXX_STANDARD 23)

target_compile_definitions(fish_engine PUBLIC GLFW_INCLUDE_NONE )
target_include_directories(fish_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(fish_engine PUBLIC fish::common glfw glad EnTT::EnTT glm::glm)