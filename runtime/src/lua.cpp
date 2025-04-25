#include "fish/lua.hpp"
#include "fish/assets.hpp"
#include "fish/console.hpp"
#include "fish/engineinfo.hpp"
#include "fish/events.hpp"
#include "fish/lua/luaworld.hpp"
#include "fish/services.hpp"
#include "fish/helpers.hpp"
#include "fish/world.hpp"
#include <filesystem>
#include <format>
#include <set>
#include <sol/forward.hpp>
#include <sol/sol.hpp>
#include <sol/string_view.hpp>
#include <sol/table.hpp>
#include <sol/types.hpp>
#include <string>

namespace fish
{
    LuaService::LuaService(Services& services)
        : services(services), assets(services.getService<Assets>()), luaWorld(services.getService<World>())
    {}

    LuaService::LuaService(const LuaService& other)
        : services(other.services), assets(other.assets), luaWorld(other.luaWorld), luaState()
    {}

    void LuaService::init()
    {
        if (!assets.isDirectory("lua/main"))
            helpers::fatalError("assets/lua/main does not exist");
        luaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);
        this->initWorld();
        this->initEngineInfo();
        this->initEvents();
        this->initConsole();
        this->initImGui();

        auto files = assets.listDirectory("lua/main");
        
        for (auto const& file : files)
            this->runFile(file);
    }

    void LuaService::runFile(const std::filesystem::path& file)
    {
        if (!assets.isFile(file))
            helpers::fatalError(std::format("Tried to run invalid lua file {}", file.string()));

        std::string code = assets.findAssetString(file);

        this->luaState.safe_script(code, file.string());
    }

    void LuaService::update()
    {
        auto& engineInfo = this->services.getService<EngineInfo>();

        auto engineInfoLua = luaState["EngineInfo"].get_or_create<sol::table>();
        engineInfoLua["deltaTime"] = engineInfo.deltaTime;
        engineInfoLua["gameTime"] = engineInfo.gameTime;
    }

    void LuaService::initWorld()
    {
        auto& world = this->services.getService<World>();
        auto worldLua = this->luaState["World"].get_or_create<sol::table>();

        worldLua["get"] = [&]() { return this->luaWorld; };

        worldLua.new_usertype<lua::LuaEntityWrapper>("Entity",
            "isValid",
            &lua::LuaEntityWrapper::isValid,
            "destroy",
            &lua::LuaEntityWrapper::destroy,
            "getParent",
            &lua::LuaEntityWrapper::getParent
        );

        worldLua.new_usertype<lua::LuaWorld>("World",
            "create",
            &lua::LuaWorld::create
        );
    }

    void LuaService::initEngineInfo()
    {
        auto& engineInfo = this->services.getService<EngineInfo>();

        auto engineInfoLua = luaState["EngineInfo"].get_or_create<sol::table>();
        engineInfoLua["deltaTime"] = 0.0f;
        engineInfoLua["gameTime"] = 0.0f;
        engineInfoLua["tickRate"] = engineInfo.tickRate;
        engineInfoLua["runType"] = static_cast<unsigned char>(engineInfo.runType);
    }

    void LuaService::initEvents()
    {
        auto& events = this->services.getService<EventDispatcher>();

        auto luaEvents = luaState["Events"].get_or_create<sol::table>();

        luaEvents["observe"] = [&](sol::string_view nameSol, sol::function fn) {
            std::string name(nameSol);

            if (!this->luaEvents.contains(name))
                this->luaEvents[name] = {};

            this->luaEvents[name].push_back(fn);
        };

        events.observeAll([&](auto& name, auto& data) {
            if (name == "update")
                this->update();

            if (this->luaEvents.contains(name))
                for (auto const& event : this->luaEvents[name])
                    event();

            return data;
        });
    }

    void LuaService::initConsole()
    {
        auto& console = this->services.getService<Console>();
        auto luaConsole = luaState["Console"].get_or_create<sol::table>();
        
        
        luaConsole["runCommand"] = [&](sol::string_view name) {
            std::set<std::string> args;
            console.runCommand(std::string(name), args);
        };

    }

}