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
#include "fish/helpers.hpp"
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
        this->services.addService<EngineInfo>();
        this->services.addServiceData<TextureManager>(TextureManager(services));
        auto& sceneLoader = this->services.addServiceData(SceneLoader(services));
        this->services.addServiceData(UI(services));
        this->services.addServiceData(ShaderCache(services));

        // init libraries
        glfwSetErrorCallback(glfwErrorCallback);
        FISH_ASSERT(glfwInit(), "GLFW failed to initialize");
        
#ifdef FISH_STEAM
        this->initSteam();
#endif
        // init engine-related things
        this->initWindow();
        sceneLoader.init();
        world.addSystem<Renderer3D, Services&, GLFWwindow*>(this->services, window);
        world.addSystem<Renderer2D, Services&, GLFWwindow*>(this->services, window);
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
        
        auto& world = this->services.getService<World>();
        auto& shaderCache = this->services.getService<ShaderCache>();

        // destroy objects
        shaderCache.shutdown();
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
        auto& events = this->services.getService<EventDispatcher>();
        auto& userInput = this->services.getService<UserInput>();
        auto& engineInfo = this->services.getService<EngineInfo>();
        auto& ui = this->services.getService<UI>();
        auto& textureManager = this->services.getService<TextureManager>();
        auto& screen = this->services.addService<Screen>(window);
        
        float tickDelay = 1.0f / engineInfo.tickRate;
        float nextTick = 0.0f;

        ui.init();
        screen.init();
        events.dispatch("start");
        while (!glfwWindowShouldClose(window))
        {
            std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
            engineInfo.gameTime += engineInfo.deltaTime;
            std::cout << "FPS: " << 1.0f / engineInfo.deltaTime << std::endl;
            // tick
            if (nextTick < engineInfo.gameTime) {
                nextTick += tickDelay;
                textureManager.gc();
                world.tick();
            }
            screen.clearScreen();

            events.dispatch("update");
            // frame logic            
            world.update();
            glfwSwapBuffers(window);
            glfwPollEvents();

            // frame end
            std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
            engineInfo.deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0f;
        }
    }
}