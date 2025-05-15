#include "fish/lua.hpp"
#include "entt/entity/fwd.hpp"
#include "fish/assets.hpp"
#include "fish/camera.hpp"
#include "fish/common.hpp"
#include "fish/console.hpp"
#include "fish/engineinfo.hpp"
#include "fish/events.hpp"
#include "fish/lua/luaevents.hpp"
#include "fish/scenes.hpp"
#include "fish/services.hpp"
#include "fish/helpers.hpp"
#include "fish/world.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include <filesystem>
#include <format>
#include <set>
#include <sol/error.hpp>
#include <sol/forward.hpp>
#include <sol/function_types_templated.hpp>
#include <sol/lua_value.hpp>
#include <sol/optional_implementation.hpp>
#include <sol/sol.hpp>
#include <sol/string_view.hpp>
#include <sol/table.hpp>
#include <sol/types.hpp>
#include <string>

namespace fish
{
    LuaService::LuaService(Services& services)
        : services(services), assets(services.getService<Assets>())
    {}

    LuaService::LuaService(const LuaService& other)
        : services(other.services), assets(other.assets), luaState()
    {}

    void LuaService::init()
    {
        if (!assets.isDirectory("lua/main"))
            helpers::fatalError("assets/lua/main does not exist");
        luaState.open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::math,
            sol::lib::table
        );
        this->initWorld();
        this->initEngineInfo();
        this->initEvents();
        this->initConsole();
        this->initSceneLoader();
        this->initMath();
        this->initImGui();
        this->initComponents();

        auto files = assets.listDirectory("lua/main");
        
        for (auto const& file : files)
            this->runFile(file);
    }

    void LuaService::shutdown()
    {
        this->luaEvents.clear();
    }

    void LuaService::runFile(const std::filesystem::path& file)
    {
        if (!assets.isFile(file))
            helpers::fatalError(std::format("Tried to run invalid lua file {}", file.string()));

        auto asset = assets.findAsset(file);

        auto result = this->luaState.safe_script(asset->str(), file.string());
        FISH_ASSERTF(result.valid(), "Main script did not execute correctly!: {}", static_cast<sol::error>(result).what());
    }

    std::string LuaService::toString(sol::lua_value value)
    {
        sol::function tostring = this->luaState["tostring"];
        return tostring(value);
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

        worldLua["get"] = [&]() -> World& { return world; };

        worldLua.new_usertype<entt::entity>("Entity");

        worldLua.new_usertype<World>("World",
            "create",
            [&](World& self, sol::optional<entt::entity> parent) {
                if (parent.has_value())
                    return self.create(parent.value());

                return self.create();
            },
            "getCamera",
            [&](World& self) { return self.getCamera(); },
            "setCamera",
            [&](World& self, entt::entity entity) { self.setCamera(entity); },
            "isValid",
            [&](World& self, entt::entity entity) { return self.isValid(entity); },
            "destroy",
            [&](World& self, entt::entity entity) { return self.destroy(entity); },
            "getParent",
            [&](World& self, entt::entity entity) { return self.getParent(entity); }
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
        luaEvents.new_usertype<lua::LuaEventData>("EventData",
            "getString",
            &lua::LuaEventData::getString
        );

        luaEvents["observe"] = [&](sol::string_view nameSol, sol::function fn) {
            std::string name(nameSol);

            if (!this->luaEvents.contains(name))
                this->luaEvents[name] = {};

            this->luaEvents[name].push_back(fn);
        };

        events.observeAll([&](auto& name, auto& data) {
            if (name == "update")
                this->update();

            lua::LuaEventData luaEvent(data);

            if (this->luaEvents.contains(name))
                for (auto const& event : this->luaEvents[name])
                    event(luaEvent);

            return data;
        });
    }

    void LuaService::initSceneLoader()
    {
        auto& sceneLoader = this->services.getService<SceneLoader>();
        auto& assets = this->services.getService<Assets>();

        auto luaSceneLoader = luaState["SceneLoader"].get_or_create<sol::table>();
    }

    void LuaService::initMath()
    {
        luaState.new_usertype<glm::vec3>("Vec3",
            "x",
            &glm::vec3::x,
            "y",
            &glm::vec3::y,
            "z",
            &glm::vec3::z
        );

        luaState.new_usertype<glm::quat>("Quaternion",
            "x",
            &glm::quat::x,
            "y",
            &glm::quat::y,
            "z",
            &glm::quat::z,
            "w",
            &glm::quat::w
        );
    }

    void LuaService::initConsole()
    {
        auto& console = this->services.getService<Console>();
        auto luaConsole = luaState["Console"].get_or_create<sol::table>();
        
        luaConsole["log"] = [&](std::string message) {
            console.log(message);
        };

        luaConsole["getLogs"] = [&]() {
            const auto& logs = console.getLogs();

            sol::table tbl = luaState.create_table(logs.size());
            for (auto const& log : logs)
                tbl.add(log);

            return tbl;
        };

        luaConsole["runCommand"] = [&](std::string name, sol::optional<sol::table> argTable) {
            std::set<std::string> args;

            if (argTable.has_value()) {
                for (auto [key, value] : argTable.value())
                    if (value.is<std::string>())
                        args.insert(value.as<std::string>());
                    else
                        args.insert(toString(value));
            }

            console.runCommand(name, args);
        };

        luaConsole["registerCommand"] = [&](std::string name, sol::function func) {
            console.registerCommand(name, [&, func](std::set<std::string> args) {
                sol::table commandArgTable = luaState.create_table(args.size());

                for (std::string arg : args)
                    commandArgTable.add(arg);

                func(commandArgTable);
            });
        };
    }

    void LuaService::initComponents()
    {
        {
            sol::constructors<Camera3D(float, float)> ctor = {};
            luaState.new_usertype<Camera3D>("Camera3D",
                ctor,
                "fov",
                &Camera3D::fov,
                "zNear",
                &Camera3D::zNear
            );
        }
    }
}