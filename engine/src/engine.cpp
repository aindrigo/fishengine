#include <cpptrace/basic.hpp>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <steam/steam_api_common.h>
#include <string>
#include "fish/engine.hpp"
#include "GLFW/glfw3.h"
#include "fish/assets.hpp"
#include "fish/common.hpp"
#include "fish/console.hpp"
#include "fish/events.hpp"
#include "fish/helpers.hpp"
#include "fish/imguisystem.hpp"
#include "fish/renderer.hpp"
#include "fish/scenes.hpp"
#include "fish/services.hpp"
#include "fish/texture.hpp"
#include "fish/ui.hpp"
#include "fish/userinput.hpp"
#include "fish/world.hpp"
#include "fish/engineinfo.hpp"
#include "glad/gl.h"
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
     
        // services
        auto& world = this->services.addService<World>();
        this->services.addService<Assets>(std::filesystem::current_path() / "assets");
        auto& engineInfo = this->services.addServiceData<EngineInfo>(EngineInfo { .runType = initData.runType });
        auto& sceneLoader = this->services.addServiceData(SceneLoader(services));
        auto& console = this->services.addServiceData(Console(services));

        if (engineInfo.runType == EngineRunType::NORMAL) {
            this->services.addServiceData<TextureManager>(TextureManager(services));
            this->services.addServiceData(UI(services));
            this->services.addServiceData(ShaderManager(services));

            // init rendering-related things
            glfwSetErrorCallback(glfwErrorCallback);
            FISH_ASSERT(glfwInit(), "GLFW failed to initialize");

            this->initWindow();
            glEnable(GL_DEPTH_TEST);
            glfwSwapInterval(1);
            glfwShowWindow(window);
        }
        
        // init libraries
#ifdef FISH_STEAM
        this->initSteam();
#endif
        
        // base console commands
        console.registerCommand("quit", [this](auto& args) {
            this->stop();
            exit(EXIT_SUCCESS);
        });

        // start loop
        sceneLoader.init();
        console.init();
        doLoop();
    }

    void Engine::stop()
    {
        FISH_ASSERTF(this->state != EngineState::NOT_RUNNING, "{} is not running", initData.name);
        this->state = EngineState::NOT_RUNNING;
        
        auto& engineInfo = this->services.getService<EngineInfo>();
        // shutdown services
        auto& world = this->services.getService<World>();
        world.shutdown();

        auto& console = this->services.getService<Console>();
        console.shutdown();

        // destroy rendering-related stuff
        if (engineInfo.runType == EngineRunType::NORMAL) {
            auto& shaderCache = this->services.getService<ShaderManager>();
            shaderCache.shutdown();
            glfwDestroyWindow(window);
            glfwTerminate();
        }
        
        // terminate libraries
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
            exit(EXIT_SUCCESS); // i'd end it all for you, Gabe...
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
            initData.width, initData.height, 
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

    bool Engine::shouldLoopContinue()
    {
        auto& engineInfo = this->services.getService<EngineInfo>();
        if (engineInfo.runType == EngineRunType::NORMAL)
            return !glfwWindowShouldClose(window);

        return true;
    }

    void Engine::doLoop()
    {
        auto& world = this->services.getService<World>();
        auto& events = this->services.getService<EventDispatcher>();
        auto& engineInfo = this->services.getService<EngineInfo>();
        auto& console = this->services.getService<Console>();
        float tickDelay = 1.0f / engineInfo.tickRate;
        float nextTick = 0.0f;

        if (engineInfo.runType == EngineRunType::NORMAL) {
            auto& screen = this->services.addService<Screen>(window);
            auto& userInput = this->services.getService<UserInput>();
            auto& ui = this->services.getService<UI>();
            auto& textureManager = this->services.getService<TextureManager>();
            world.addSystem<Renderer3D, Services&>(this->services, window);
            world.addSystem<Renderer2D, Services&>(this->services, window);
            world.addSystem<ImGuiSystem, Services&>(this->services, window);
            ui.init();
            screen.init();
        }

        events.dispatch("start");
        while (shouldLoopContinue())
        {
            std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
            engineInfo.gameTime += engineInfo.deltaTime;
            if (nextTick < engineInfo.gameTime) {
                nextTick += tickDelay;
                events.dispatch("tick");
                world.tick();
            }

            // update
            events.dispatch("update");
            world.update();
            console.update();

            // frame logic
            if (engineInfo.runType == EngineRunType::NORMAL) {
                auto& screen = services.getService<Screen>();
                screen.clearScreen();
                world.render();

                glfwSwapBuffers(window);
                glfwPollEvents();
            }

            // frame end
            std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
            engineInfo.deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0f;
        }
    }
}