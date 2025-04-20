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
        glfwSetKeyCallback(window, UserInput::callbackKeyPress);
    }

    void UserInput::setCursorLockMode(CursorLockMode mode)
    {
        int inputMode;
        switch (mode) {
            case CursorLockMode::DISABLED:
                inputMode = GLFW_CURSOR_DISABLED;
                break;
            case CursorLockMode::NORMAL:
                inputMode = GLFW_CURSOR_NORMAL;
                break;
        }

        glfwSetInputMode(window, GLFW_CURSOR, inputMode);
    }

    glm::vec2 UserInput::getCursorPos()
    {
        return { this->cursorX, this->cursorY };
    }

    bool UserInput::isKeyDown(int key)
    {
        return glfwGetKey(window, key);
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
    
    void UserInput::callbackKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        UserInput* ptr = reinterpret_cast<UserInput*>(glfwGetWindowUserPointer(window));
        FISH_ASSERT(ptr != nullptr, "GLFWwindow has invalid userpointer");

        ptr->onKeyPress(key, action);
    }

    void UserInput::onCursorPosChange(double x, double y)
    {
        EventData data;
        data.setProperty("delta", glm::vec2(x - this->cursorX, y - this->cursorY));

        auto& dispatcher = this->services.getService<EventDispatcher>();
        dispatcher.dispatch("onCursorMove", data);

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

    void UserInput::onKeyPress(int button, int action)
    {
        if (action == GLFW_PRESS && !this->keysPressed.contains(button)) {
            this->keysPressed[button] = true;
        } else if (action == GLFW_RELEASE && this->keysPressed.contains(button)) {
            this->keysPressed.erase(button);
        }
    }
}