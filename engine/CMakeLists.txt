file(GLOB_RECURSE FISH_ENGINE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

set(FISH_ENGINE_DEPS
    fish::common
    glfw
    glad
    EnTT::EnTT
    glm::glm
    cpptrace::cpptrace
    assimp
    stb
    nlohmann_json
)

add_library(fish_engine STATIC ${FISH_ENGINE_SOURCES})
add_library(fish::engine ALIAS fish_engine)

add_dependencies(fish_engine ${FISH_ENGINE_DEPS})
set_property(TARGET fish_engine PROPERTY CXX_STANDARD 23)

target_compile_definitions(fish_engine PUBLIC GLFW_INCLUDE_NONE )
target_include_directories(fish_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(fish_engine PUBLIC ${FISH_ENGINE_DEPS})

if (WIN32)
    target_compile_definitions(fish_engine PRIVATE WIN32_LEAN_AND_MEAN)
endif()
if (STEAM)
    add_dependencies(fish_engine steamworks)
    target_compile_definitions(fish_engine PUBLIC FISH_STEAM)
    target_link_libraries(fish_engine PUBLIC steamworks)
endif()