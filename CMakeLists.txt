cmake_minimum_required(VERSION 3.15)

project(FishEngine
    VERSION 0.0.0
    LANGUAGES C CXX)


set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

function(add_vendor VENDOR_NAME)
    add_subdirectory(${VENDOR_DIR}/${VENDOR_NAME})
endfunction()

option(STEAM "Should Steamworks be included & integrated?" OFF)

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
add_vendor(stb)
add_vendor(imgui)
add_vendor(sol2)
add_vendor(luajit)

add_subdirectory(${PROJECT_SOURCE_DIR}/common)
add_subdirectory(${PROJECT_SOURCE_DIR}/engine)
add_subdirectory(${PROJECT_SOURCE_DIR}/runtime)