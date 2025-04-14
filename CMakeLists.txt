cmake_minimum_required(VERSION 3.15)

project(FishEngine
    VERSION 0.0.0
    LANGUAGES C CXX)

set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

function(add_vendor VENDOR_NAME)
    add_subdirectory(${VENDOR_DIR}/${VENDOR_NAME})
endfunction()

add_vendor(glfw)
add_vendor(glad)

add_subdirectory(${PROJECT_SOURCE_DIR}/common)
add_subdirectory(${PROJECT_SOURCE_DIR}/engine)
add_subdirectory(${PROJECT_SOURCE_DIR}/runtime)