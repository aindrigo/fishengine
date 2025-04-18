#include "fish/runtime.hpp"
#include "fish/engine.hpp"
#include "fish/engineinfo.hpp"
#include "fish/events.hpp"
#include "fish/helpers.hpp"
#include "fish/scenes.hpp"
#include "fish/transform.hpp"
#include "fish/world.hpp"
#include "fish/runtimesystem.hpp"
#include "glm/ext/vector_float3.hpp"
#include "nlohmann/json.hpp" // IWYU pragma: keep
#include "nlohmann/json_fwd.hpp"
#include <filesystem>
#include <optional>
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
            auto& loader = services.getService<SceneLoader>();
            auto& engineInfo = services.getService<EngineInfo>();

            auto& registry = world.getRegistry();

            auto& camera = engineInfo.camera.value();
            auto& transform = registry.get<Transform3D>(camera);

            transform.eulerAngles.y = 89.5f;
            transform.position.y = -500.0f;
            transform.position.z = -900.0f;
            Scene scene = loader.load("models/sponza.obj");
            loader.loadIntoWorld(scene);

            world.addSystem<RuntimeSystem>();

            // events.observe("onCursorMove", [&](auto& cursorEventData) {
            //     std::optional<glm::vec2> deltaOpt = cursorEventData.template getProperty<glm::vec2>("delta");
            //     if (!deltaOpt.has_value())
            //         return eventData;

            //     glm::vec2 delta = deltaOpt.value() * 0.01f;
            //     transform.eulerAngles += glm::vec3(delta.y, delta.x, 0);
            //     return eventData;
            // });
            
            return eventData;
        });
        this->engine->start();
        this->engine->stop();
    }
}