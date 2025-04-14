#include <format>
#include <iostream>
#include <string>
#include "fish/game.hpp"
#include "GLFW/glfw3.h"
#include "fish/common.hpp"
#include "glad/gl.h"

void glfwErrorCallback(int code, const char* msg)
{
    std::cout << std::format("GLFW error {} caught: {}", code, msg) << std::endl;
}

namespace fish
{
    Game::Game(const std::string& name, const Version& version)
        : name(name), version(version)
    {
    }

    void Game::start()
    {
        FISH_ASSERTF(this->state == GameState::NOT_RUNNING, "{} is already running", name);
        this->state = GameState::RUNNING;
        
        glfwSetErrorCallback(glfwErrorCallback);
        FISH_ASSERT(glfwInit(), "GLFW failed to initialize");

        this->initWindow();

        while (!glfwWindowShouldClose(window))
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    void Game::stop()
    {
        FISH_ASSERTF(this->state != GameState::NOT_RUNNING, "{} is not running", name);
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    
    void Game::initWindow()
    {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        this->window = glfwCreateWindow(
            800, 600, 
            std::format("{} v{}", name, version.str()).c_str(), 
            nullptr, nullptr
        );
        
        glfwMakeContextCurrent(window);
        gladLoadGL(glfwGetProcAddress);

        glfwShowWindow(window);
    }
}