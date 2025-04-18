cmake_minimum_required(VERSION 3.15)

project(FishEngine
    VERSION 0.0.0
    LANGUAGES C CXX)


set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

function(add_vendor VENDOR_NAME)
    add_subdirectory(${VENDOR_DIR}/${VENDOR_NAME})
endfunction()

set(STEAM ON)
option(STEAM "Should Steamworks be included & integrated?" ON)

set(ASSIMP_INSTALL OFF CACHE BOOL "disable assimp install")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "disable assimp tests")
set(ASSIMP_NO_EXPORT OFF CACHE BOOL "disable assimp export")

add_vendor(glfw)
add_vendor(glad)
add_vendor(entt)
add_vendor(glm)
add_vendor(cpptrace)
add_vendor(steamworks)
add_vendor(json)
add_vendor(assimp)

add_subdirectory(${PROJECT_SOURCE_DIR}/common)
add_subdirectory(${PROJECT_SOURCE_DIR}/engine)
add_subdirectory(${PROJECT_SOURCE_DIR}/runtime)