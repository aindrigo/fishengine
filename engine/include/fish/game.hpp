#pragma once

#include <string>
#include "GLFW/glfw3.h"
#include "fish/version.hpp"

namespace fish
{
    
    class Game
    {
    public:
        Game(const std::string& name, const Version& version);

        void start();
        void stop();
    private:
        void initWindow();
        enum struct GameState
        {
            RUNNING,
            NOT_RUNNING
        };

        GameState state = GameState::NOT_RUNNING;
        GLFWwindow* window;

        const std::string name;
        const Version version;
    };
}