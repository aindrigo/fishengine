#pragma once

#include "GLFW/glfw3.h"
#include "fish/services.hpp"
#include "fish/system.hpp"
#include "fish/world.hpp"

namespace fish
{
    class ImGuiSystem : public ISystem
    {
    public:
        ImGuiSystem(Services& services, GLFWwindow* window);

        void init() override;
        void render() override;
        void shutdown() override;
    private:
        GLFWwindow* window;
        World& world;
        Services& services;
    };
}