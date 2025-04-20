#pragma once

#include "GLFW/glfw3.h"
#include "fish/services.hpp"
#include "glm/ext/vector_float2.hpp"
#include <unordered_map>
namespace fish
{
    enum struct CursorLockMode {
        DISABLED,
        NORMAL
    };

    class UserInput
    {
    public:
        UserInput(GLFWwindow* window, Services& services);
        void installCallbacks();
        void setCursorLockMode(CursorLockMode lockMode);
        glm::vec2 getCursorPos();

        bool isKeyDown(int key);
    private:
        static void callbackCursorPos(GLFWwindow* window, double x, double y);
        static void callbackMouseBtn(GLFWwindow* window, int button, int action, int mods);
        static void callbackKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods);
        void onCursorPosChange(double x, double y);
        void onMouseButton(int button, int action);
        void onKeyPress(int key, int action);

        double cursorX;
        double cursorY;
        GLFWwindow* window;
        Services& services;
        std::unordered_map<int, bool> keysPressed;
    };
}