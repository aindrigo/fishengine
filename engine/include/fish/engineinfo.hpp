#pragma once

namespace fish
{
    enum struct EngineRunType : unsigned char {
        NORMAL = 0,
        HEADLESS = 1
    };

    struct EngineInfo {
        const float tickRate = 64.0f;
        const EngineRunType runType;
        float gameTime = 0.0f;
        float deltaTime = 0.0f;
    };
}