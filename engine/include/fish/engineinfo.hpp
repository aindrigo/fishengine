#pragma once

#include "entt/entity/fwd.hpp"
#include <optional>
namespace fish
{
    enum struct EngineRunType {
        NORMAL,
        HEADLESS
    };

    struct EngineInfo {
        const float tickRate = 64.0f;
        const EngineRunType runType;
        float gameTime = 0.0f;
        float deltaTime = 0.0f;
        std::optional<entt::entity> camera;
    };
}