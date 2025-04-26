#pragma once

#include "fish/assets.hpp"
#include "lua/luaworld.hpp"
#include <filesystem>
#include <sol/forward.hpp>
#include <sol/lua_value.hpp>
#include <sol/optional_implementation.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#define SOL_ALL_SAFETIES_ON 1

#include "fish/services.hpp"
#include <sol/state.hpp>

namespace fish
{
    class LuaService
    {
    public:
        LuaService(Services& services);
        LuaService(const LuaService& other);

        void init();
        void shutdown();
        void runFile(const std::filesystem::path& path);
        std::string toString(sol::lua_value value);
    private:
        void update();
        void initWorld();
        void initEngineInfo();
        void initEvents();
        void initConsole();
        void initSceneLoader();
        void initMath();
        void initImGui();

        Services& services;
        Assets& assets;
        std::unordered_map<std::string, std::vector<sol::protected_function>> luaEvents;
        lua::LuaWorld luaWorld;
        sol::state luaState;
    };
}