#pragma once

#include "GLFW/glfw3.h"
#include "glm/ext/vector_float2.hpp"
namespace fish
{
    class UserInput
    {
    public:
        UserInput(GLFWwindow* window);
        void installCallbacks();

        glm::vec2 getCursorPos();
    private:
        static void callbackCursorPos(GLFWwindow* window, double x, double y);
        void onCursorPosChange(double x, double y);

        double cursorX;
        double cursorY;
        GLFWwindow* window;
    };
}