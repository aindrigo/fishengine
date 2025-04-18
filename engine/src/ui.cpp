#include "fish/ui.hpp"
#include "entt/entity/fwd.hpp"
#include "fish/services.hpp"
#include "GLFW/glfw3.h"
#include "fish/transform.hpp"
#include "fish/world.hpp"
#include "fish/userinput.hpp"
#include "fish/events.hpp"
#include "fish/model.hpp"
#include "fish/helpers.hpp"
#include "glm/ext/vector_float2.hpp"
#include <optional>

namespace fish
{
    // UI
    UI::UI(Services& services)
        : services(services)
    {}

    void UI::init()
    {
        auto& events = services.getService<EventDispatcher>();
        auto& world = services.getService<World>();
        auto& userInput = services.getService<UserInput>();

        events.observe("onClick", [&](EventData& data) {
            std::optional<int> button = data.getProperty<int>("button");
            std::optional<int> action = data.getProperty<int>("action");

            if (!button.has_value() || !action.has_value())
                return data;

            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                glm::vec2 mousePos = userInput.getCursorPos();
                auto group = world.getRegistry().group(entt::get<Panel, Transform2D>);

                group.sort<Transform2D>([](const Transform2D& lhs, const Transform2D& rhs) {
                    return lhs.z > rhs.z;
                });

                for (auto ent : group) {
                    auto [panel, transform] = group.get(ent);
                    if (!panel.movable)
                        continue;

                    auto bounds = world.worldSpace2DTransform(ent).bounds();
                    //printf("(%f %f) (%f %f) %f %f\n", bounds.first.x, bounds.first.y, bounds.second.x, bounds.second.y, mousePos.x, mousePos.y);
                    if (helpers::Math::inBounds(mousePos, bounds)) {
                        this->onPanelClick(ent, world, mousePos, panel);
                        break;
                    }
                }
            } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && this->cursor.moving) {
                this->cursor.moving = false;
                this->cursor.panel = std::nullopt;
            }
            return data;
        });

        events.observe("onCursorMove", [&](EventData& data) {
            if (!this->cursor.moving)
                return data;

            std::optional<glm::vec2> deltaOpt = data.getProperty<glm::vec2>("delta");
            if (!deltaOpt || !cursor.panel)
                return data;

            glm::vec2 delta = deltaOpt.value();
            
            entt::entity ent = cursor.panel.value();

            auto& transform = world.getRegistry().get<Transform2D>(ent);
            transform.position += delta;

            return data;
        });
    }
    
    void UI::onPanelClick(entt::entity panelEntity, World& world, glm::vec2 mousePos, Panel& panel)
    {
        auto& transform = world.getRegistry().get<Transform2D>(panelEntity);

        this->cursor.moving = true;
        this->cursor.panel = panelEntity;
    }
}