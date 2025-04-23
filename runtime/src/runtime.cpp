#include "fish/runtime.hpp"
#include "fish/camera.hpp"
#include "fish/engine.hpp"
#include "fish/engineinfo.hpp"
#include "fish/events.hpp"
#include "fish/helpers.hpp"
#include "fish/lights.hpp"
#include "fish/scenes.hpp"
#include "fish/transform.hpp"
#include "fish/userinput.hpp"
#include "fish/world.hpp"
#include "fish/runtimesystem.hpp"
#include "glm/common.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/trigonometric.hpp"
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
        glm::vec2 mouseRot { 0, 0 };

        events.observe("start", [&](auto& eventData) {
            auto& world = services.getService<World>();
            auto& loader = services.getService<SceneLoader>();
            auto& engineInfo = services.getService<EngineInfo>();
            auto& userInput = services.getService<UserInput>();

            auto& registry = world.getRegistry();
            
            auto camera = world.create();
            registry.emplace<Camera3D>(camera);
            auto& transform = registry.emplace<Transform3D>(camera);

            engineInfo.camera = camera;
            //transform.rotation.y = 90.0f;
            //transform.position.y = -500.0f;
            //transform.position.z = -800.0f;
            Scene scene = loader.load("models/sponza.obj");
            loader.loadIntoWorld(scene);

            userInput.setCursorLockMode(CursorLockMode::DISABLED);

            world.addSystem<RuntimeSystem>();

            auto light = world.create();
            
            registry.emplace<PointLight>(light, PointLight {
                .color = { 255, 255, 255, 255 }
            });
            auto& lightTransform = registry.emplace<Transform3D>(light);
            lightTransform.position.z -= 5;

            // auto dirLight = world.create();
            // registry.emplace<DirectionalLight>(dirLight, DirectionalLight {
            //     .direction = { 0.0f, -1.0f, 0.5f }
            // });

            events.observe("onCursorMove", [&](auto& cursorEventData) {
                std::optional<glm::vec2> deltaOpt = cursorEventData.template getProperty<glm::vec2>("delta");
                if (!deltaOpt.has_value())
                    return eventData;

                glm::vec2 delta = deltaOpt.value() * 0.2f;

                mouseRot += glm::vec2(delta.x, delta.y);
                mouseRot.y = glm::clamp(mouseRot.y, -90.0f, 90.0f);
                transform.rotation = glm::angleAxis(glm::radians(mouseRot.y), glm::vec3(1.0f, 0.0f, 0.0f)) *
                                      glm::angleAxis(glm::radians(mouseRot.x), glm::vec3(0.0f, 1.0f, 0.0f));
                
                return eventData;
            });

            events.observe("update", [&](auto& updateEventData) {
                float speed = 285.0f;
                glm::vec3 move { 0, 0, 0 };
                if (userInput.isKeyDown(GLFW_KEY_W))
                    move.z += speed;
                if (userInput.isKeyDown(GLFW_KEY_S))
                    move.z -= speed;
                if (userInput.isKeyDown(GLFW_KEY_A))
                    move.x += speed;
                if (userInput.isKeyDown(GLFW_KEY_D))
                    move.x -= speed;
                if (userInput.isKeyDown(GLFW_KEY_SPACE))
                    move.y -= speed;
                if (userInput.isKeyDown(GLFW_KEY_LEFT_SHIFT))
                    move.y += speed;
                
                if (move.x != 0 || move.y != 0 || move.z != 0) {
                    auto forward = transform.forward();
                    auto right = transform.right();
                    auto up = glm::vec3(0, 1, 0);
                    auto move3d = forward * move.z + right * move.x + up * move.y;

                    transform.position += engineInfo.deltaTime * move3d;

                    std::cout << std::format("{} {} {}", transform.position.x, transform.position.y, transform.position.z) << std::endl;
                }
                return updateEventData;
            });
            
            return eventData;
        });
        this->engine->start();
        this->engine->stop();
    }
}