#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include "fish/game.hpp"
#include "GLFW/glfw3.h"
#include "fish/assets.hpp"
#include "fish/common.hpp"
#include "fish/material.hpp"
#include "fish/mesh.hpp"
#include "fish/renderer.hpp"
#include "glad/gl.h"
#include <chrono>
void glfwErrorCallback(int code, const char* msg)
{
    std::cout << std::format("GLFW error {} caught: {}", code, msg) << std::endl;
}

namespace fish
{
    Game::Game(const std::string& name, const Version& version)
        : name(name), version(version), assets(Assets(std::filesystem::current_path() / "assets"))
    {
        this->world = std::make_unique<World>();
    }

    void Game::start()
    {
        FISH_ASSERTF(this->state == GameState::NOT_RUNNING, "{} is already running", name);
        this->state = GameState::RUNNING;
        
        // init libraries
        glfwSetErrorCallback(glfwErrorCallback);
        FISH_ASSERT(glfwInit(), "GLFW failed to initialize");

        // init engine-related things
        this->initWindow();

        this->world->addSystem<Renderer, Assets&>(this->assets);
        // start loop
        glfwSwapInterval(1);
        glfwShowWindow(window);

        auto registry = world->getRegistry().lock();

        auto ent = registry->create();
        registry->emplace<Mesh>(ent, Mesh {
            .vertices = { 
                0.5f,  0.5f, 0.0f, 
                0.5f, -0.5f, 0.0f,  
                -0.5f, -0.5f, 0.0f, 
                -0.5f,  0.5f, 0.0f
            },
            .indices = {
                0, 1, 3,
                1, 2, 3
            }
        });
        registry->emplace<Material>(ent, Material {
            .shader = "TestShader"
        });
        
        while (!glfwWindowShouldClose(window))
        {
            //std::cout << "FPS : " << 1 / deltaTime << std::endl;
            std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
            
            // frame logic
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            this->world->update();

            glfwSwapBuffers(window);
            glfwPollEvents();
            
            // frame end
            std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
            deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0f;
        }
    }

    void Game::stop()
    {
        FISH_ASSERTF(this->state != GameState::NOT_RUNNING, "{} is not running", name);
        this->state = GameState::NOT_RUNNING;
        
        // destroy objects
        this->world->shutdown();
        glfwDestroyWindow(window);

        // terminate libraries
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
    }
}