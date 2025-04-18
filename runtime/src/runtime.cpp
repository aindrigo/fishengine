#include "fish/runtime.hpp"
#include "fish/engine.hpp"
#include "fish/events.hpp"
#include "fish/helpers.hpp"
#include "fish/transform.hpp"
#include "fish/world.hpp"
#include "fish/runtimesystem.hpp"
#include "nlohmann/json.hpp" // IWYU pragma: keep
#include "nlohmann/json_fwd.hpp"
#include <filesystem>
#include <string>

namespace fish
{
    Runtime::Runtime()
    {
        std::filesystem::path gamejsonPath = std::filesystem::current_path() / "game.json";
        nlohmann::json json = nlohmann::json::parse(helpers::File::readFile(gamejsonPath));

        nlohmann::json nameJson = json.at("name");
        FISH_ASSERT(nameJson.is_string(), "(game.json).name was not provided, or is not a string.");
        this->name = nameJson;
        
        nlohmann::json versionJson = json.at("version");
        FISH_ASSERT(versionJson.is_structured(), "(game.json).version was not provided, or is not a table.");

        nlohmann::json versionMajorJson = versionJson.at("major");
        FISH_ASSERT(versionMajorJson.is_number_unsigned(), "(game.json).version.major was not provided, or is not a positive number.");

        nlohmann::json versionMinorJson = versionJson.at("minor");
        FISH_ASSERT(versionMinorJson.is_number_unsigned(), "(game.json).version.minor was not provided, or is not a positive number.");

        nlohmann::json versionRevisionJson = versionJson.at("revision");
        FISH_ASSERT(versionRevisionJson.is_number_unsigned(), "(game.json).version.revision was not provided, or is not a positive number.");


        version = {
            .major = versionMajorJson,
            .minor = versionMinorJson,
            .revision = versionRevisionJson
        };

        EngineInitData data = {
            .name = name,
            .version = version
        };

#ifdef FISH_STEAM
        nlohmann::json steamJson = json.at("steam");
        FISH_ASSERT(steamJson.is_structured(), "(game.json).steam was not provided, or is not a table.");

        nlohmann::json steamAppIdJson = steamJson.at("appid");
        FISH_ASSERT(steamAppIdJson.is_number_unsigned(), "(game.json).steam.appid was not provided, or is not a number");

        data.steamAppId = steamAppIdJson;
#endif
        this->engine = std::make_unique<Engine>(data);
    }
        
    void Runtime::run()
    {
        auto& services = this->engine->getServices();
        auto& events = services.getService<EventDispatcher>();

        events.observe("start", [&](auto& eventData) {
            auto& world = services.getService<World>();

            world.addSystem<RuntimeSystem>();

            auto panel1 = world.createPanel();
            auto [material1, transform1] = world.getRegistry().get<Material, Transform2D>(panel1);
            transform1.z = 1;
            transform1.position = { 100, 100 };
            transform1.alignment = Transform2D::AlignmentMode::CENTER;
            
            auto panel2 = world.createPanel();
            auto [material2, transform2] = world.getRegistry().get<Material, Transform2D>(panel2);
            material2.setProperty("color", Color { 155, 155, 155, 200 });
            transform2.size = { 200, 200};
            transform2.position = { 0, 0 };
            transform2.z = 0;

            world.setParent(panel1, panel2);
            return eventData;
        });
        this->engine->start();
        this->engine->stop();
    }
}