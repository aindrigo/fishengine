#pragma once

#include "GLFW/glfw3.h"
#include "fish/services.hpp"
#include "glm/ext/vector_float2.hpp"
namespace fish
{
    class UserInput
    {
    public:
        UserInput(GLFWwindow* window, Services& services);
        void installCallbacks();

        glm::vec2 getCursorPos();
    private:
        static void callbackCursorPos(GLFWwindow* window, double x, double y);
        static void callbackMouseBtn(GLFWwindow* window, int button, int action, int mods);
        void onCursorPosChange(double x, double y);
        void onMouseButton(int button, int action);

        double cursorX;
        double cursorY;
        GLFWwindow* window;
        Services& services;
    };
}