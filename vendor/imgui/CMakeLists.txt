set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)

set(IMGUI_DEPS
    glfw
)
add_library(imgui STATIC ${IMGUI_SOURCES})
add_dependencies(imgui ${IMGUI_DEPS})

target_link_libraries(imgui PUBLIC ${IMGUI_DEPS})
target_include_directories(imgui PUBLIC ${IMGUI_DIR})
