#include <cpptrace/basic.hpp>
#include <filesystem>
#include <format>
#include <iostream>
#include <steam/steam_api_common.h>
#include <string>
#include "fish/engine.hpp"
#include "GLFW/glfw3.h"
#include "fish/assets.hpp"
#include "fish/common.hpp"
#include "fish/events.hpp"
#include "fish/renderer.hpp"
#include "fish/scenes.hpp"
#include "fish/services.hpp"
#include "fish/userinput.hpp"
#include "fish/world.hpp"
#include "glad/gl.h"
#include <chrono>

#ifdef FISH_STEAM
#include "steam/steam_api.h"
#endif

void glfwErrorCallback(int code, const char* msg)
{
    std::cout << std::format("GLFW error {} caught: {}", code, msg) << std::endl;
}

#ifndef NDEBUG
void glDebugOutput(GLenum source, 
                    GLenum type, 
                    unsigned int id, 
                    GLenum severity, 
                    GLsizei length, 
                    const char *message, 
                    const void *userParam)
{
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 
    std::cout << "OpenGL debug message caught (" << id << "): " << message << std::endl;
    auto trace = cpptrace::generate_trace();
    trace.print();
}
#endif

namespace fish
{
    Engine::Engine(const EngineInitData& data)
        : initData(data)
    {
        this->services.addService<EventDispatcher>();
    }

    void Engine::start()
    {
        FISH_ASSERTF(this->state == EngineState::NOT_RUNNING, "{} is already running", initData.name);
        this->state = EngineState::RUNNING;
     
        auto& world = this->services.addService<World>();
        this->services.addService<Assets>(std::filesystem::current_path() / "assets");
        this->services.addServiceData(SceneLoader(services));
        // init libraries
        glfwSetErrorCallback(glfwErrorCallback);
        FISH_ASSERT(glfwInit(), "GLFW failed to initialize");
        
#ifdef FISH_STEAM
        this->initSteam();
#endif
        // init engine-related things
        this->initWindow();

        world.addSystem<Renderer, Services&, GLFWwindow*>(this->services, window);
        // start loop
        glEnable(GL_DEPTH_TEST);
        glfwSwapInterval(1);
        glfwShowWindow(window);

        doLoop();
    }

    void Engine::stop()
    {
        FISH_ASSERTF(this->state != EngineState::NOT_RUNNING, "{} is not running", initData.name);
        this->state = EngineState::NOT_RUNNING;
        
        World& world = this->services.getService<World>();

        // destroy objects
        world.shutdown();
        glfwDestroyWindow(window);

        // terminate libraries
        glfwTerminate();
        SteamAPI_Shutdown();
    }
    
    Services& Engine::getServices()
    {
        return this->services;
    }
    
#ifdef FISH_STEAM
    void Engine::initSteam()
    {
        if (SteamAPI_RestartAppIfNecessary(initData.steamAppId))
            exit(EXIT_SUCCESS);
        SteamAPI_Init();
    }
#endif
    void Engine::initWindow()
    {
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        this->window = glfwCreateWindow(
            800, 600, 
            std::format("{} v{}", initData.name, initData.version.str()).c_str(), 
            nullptr, nullptr
        );
        
        glfwMakeContextCurrent(window);
        gladLoadGL(glfwGetProcAddress);
        auto& input = this->services.addServiceData(UserInput(window, services));
        input.installCallbacks();

#ifndef NDEBUG
        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }
#endif
    }

    void Engine::doLoop()
    {
        auto& world = this->services.getService<World>();
        auto& engineInfo = this->services.addService<EngineInfo>();

        float tickDelay = 1.0f / engineInfo.tickRate;
        float nextTick = 0.0f;

        auto& events = this->services.getService<EventDispatcher>();
        events.dispatch("start");
        while (!glfwWindowShouldClose(window))
        {
            std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
            engineInfo.gameTime += deltaTime;

            // tick
            if (nextTick < engineInfo.gameTime) {
                doTick();
                nextTick += tickDelay;
            }

            // frame logic
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            world.update();
            glfwSwapBuffers(window);
            glfwPollEvents();

            // frame end
            std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
            deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0f;
        }
    }

    void Engine::doTick()
    {
        SteamAPI_RunCallbacks();
    }
}