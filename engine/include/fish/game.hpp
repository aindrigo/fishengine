#pragma once

#include <memory>
#include <string>
#include "GLFW/glfw3.h"
#include "assets.hpp"
#include "fish/version.hpp"
#include "fish/world.hpp"

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
        Assets assets;
        std::unique_ptr<World> world;
        GLFWwindow* window;
        float deltaTime;

        const std::string name;
        const Version version;

    };
}