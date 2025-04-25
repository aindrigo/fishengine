#pragma once

#include <string>
#include "GLFW/glfw3.h"
#include "engineinfo.hpp"
#include "fish/engineinfo.hpp"
#include "fish/services.hpp"
#include "fish/version.hpp"
#include "services.hpp"

namespace fish
{
    struct EngineInitData {
        std::string name;
        EngineRunType runType = EngineRunType::NORMAL;
        Version version;
        unsigned int width = 800;
        unsigned int height = 600;
#if FISH_STEAM
        unsigned int steamAppId;
#endif
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

        bool shouldLoopContinue();
        void doLoop();
        enum struct EngineState { RUNNING, NOT_RUNNING };

        EngineState state = EngineState::NOT_RUNNING;
        Services services;
        GLFWwindow* window;

        const EngineInitData initData;

    };
}