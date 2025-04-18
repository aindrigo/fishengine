#pragma once

#include "entt/entity/fwd.hpp"
#include <optional>
namespace fish
{
    struct EngineInfo {
        const float tickRate = 64.0f;
        float gameTime = 0.0f;
        float deltaTime = 0.0f;
        std::optional<entt::entity> camera;
    };
}