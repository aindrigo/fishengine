#pragma once

#include "fish/assets.hpp"
#include "lua/luaworld.hpp"
#include <filesystem>
#include <sol/forward.hpp>
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
        void runFile(const std::filesystem::path& path);
    private:
        void update();
        void initWorld();
        void initEngineInfo();
        void initEvents();

        Services& services;
        Assets& assets;
        std::unordered_map<std::string, std::vector<sol::function>> luaEvents;
        lua::LuaWorld luaWorld;
        sol::state luaState;
    };
}