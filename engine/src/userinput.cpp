#include "fish/userinput.hpp"
#include "GLFW/glfw3.h"
#include "fish/common.hpp"
#include "fish/events.hpp"
#include "fish/services.hpp"
#include "glm/ext/vector_float2.hpp"

namespace fish
{
    
    UserInput::UserInput(GLFWwindow* window, Services& services)
        : window(window), services(services)
    {}

    void UserInput::installCallbacks()
    {
        glfwSetWindowUserPointer(window, this);
        glfwSetCursorPosCallback(window, UserInput::callbackCursorPos);
        glfwSetMouseButtonCallback(window, UserInput::callbackMouseBtn);
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

    void UserInput::callbackMouseBtn(GLFWwindow* window, int button, int action, int mods)
    {
        UserInput* ptr = reinterpret_cast<UserInput*>(glfwGetWindowUserPointer(window));
        FISH_ASSERT(ptr != nullptr, "GLFWwindow has invalid userpointer");

        ptr->onMouseButton(button, action);
    }

    void UserInput::onCursorPosChange(double x, double y)
    {
        this->cursorX = x;
        this->cursorY = y;
    }

    void UserInput::onMouseButton(int button, int action)
    {
        auto& dispatcher = this->services.getService<EventDispatcher>();

        EventData data;
        data.setProperty("button", button);
        data.setProperty("action", action);

        dispatcher.dispatch("onClick", data);
    }
}