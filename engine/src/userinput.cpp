#include "fish/userinput.hpp"
#include "GLFW/glfw3.h"
#include "fish/common.hpp"
#include "glm/ext/vector_float2.hpp"

namespace fish
{
    
    UserInput::UserInput(GLFWwindow* window)
        : window(window)
    {}

    void UserInput::installCallbacks()
    {
        glfwSetWindowUserPointer(window, this);
        glfwSetCursorPosCallback(window, UserInput::callbackCursorPos);
    }

    glm::vec2 UserInput::getCursorPos()
    {
        return { this->cursorX, this->cursorY };
    }

    void UserInput::callbackCursorPos(GLFWwindow* window, double x, double y)
    {
        UserInput* ptr = reinterpret_cast<UserInput*>(glfwGetWindowUserPointer(window));
        FISH_ASSERT(ptr != nullptr, "GLFWwindow has invalid userpointer");

        ptr->onCursorPosChange(x, y);
    }

    void UserInput::onCursorPosChange(double x, double y)
    {
        this->cursorX = x;
        this->cursorY = y;
    }
}