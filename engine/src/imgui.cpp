#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "fish/imguisystem.hpp"
#include "fish/services.hpp"
#include "fish/world.hpp"

namespace fish
{
    ImGuiSystem::ImGuiSystem(Services& services, GLFWwindow* window)
        : services(services), window(window), world(services.getService<World>())
    {}

    void ImGuiSystem::init()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 430 core");

        auto& io = ImGui::GetIO();
        io.IniFilename = 0;
        io.LogFilename = 0;
    }

    void ImGuiSystem::preRender()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiSystem::postRender()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiSystem::shutdown()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}