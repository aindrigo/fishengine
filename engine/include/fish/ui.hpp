#pragma once

#include "entt/entity/fwd.hpp"
#include "fish/model.hpp"
#include "fish/services.hpp"
#include "fish/world.hpp"
#include "glm/ext/vector_float2.hpp"
#include <optional>
namespace fish
{
    class UI
    {
    public:
        UI(Services& services);
        void init();
        void onPanelClick(entt::entity panelEntity, World& world, glm::vec2 mousePos, Panel& panel);
    private:
        struct CursorState {
            bool moving;
            glm::vec2 offset;
            std::optional<entt::entity> panel;
        };

        void update();

        CursorState cursor;
        Services& services;
    };
}