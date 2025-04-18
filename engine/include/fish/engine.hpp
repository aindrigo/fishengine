#pragma once

#include <string>
#include "GLFW/glfw3.h"
#include "fish/services.hpp"
#include "fish/version.hpp"
#include "services.hpp"

namespace fish
{
    struct EngineInitData {
        std::string name;
        Version version;
#if FISH_STEAM
        unsigned int steamAppId;
#endif
    };

    struct EngineInfo {
        const float tickRate = 64.0f;
        float gameTime = 0.0f;
        float deltaTime = 0.0f;
    };

    class Engine
    {
    public:
        Engine(const EngineInitData& data);

        void start();
        void stop();

        Services& getServices();
    private:
#ifdef FISH_STEAM
        void initSteam();
#endif
        void initWindow();
        
        void doLoop();
        void doTick();

        enum struct EngineState { RUNNING, NOT_RUNNING };

        EngineState state = EngineState::NOT_RUNNING;
        Services services;
        GLFWwindow* window;
        float deltaTime;

        const EngineInitData initData;

    };
}