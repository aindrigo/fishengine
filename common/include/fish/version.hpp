#pragma once

#include <format>
#include <string>
namespace fish
{
    enum struct Phase
    {
        STABLE,
        UNSTABLE,
        ALPHA,
        BETA,
        INDEV,
    };

    inline std::string phaseToString(const Phase& phase)
    {
        switch (phase) {
        case Phase::STABLE:
            return "stable";
        case Phase::UNSTABLE:
            return "unstable";
        case Phase::ALPHA:
            return "alpha";
        case Phase::BETA:
            return "beta";
        case Phase::INDEV:
            return "indev";
        default:
            return "unknown";
        };
    }

    struct Version
    {
        float major = 0;
        float minor = 0;
        float revision = 0;
        Phase phase = Phase::ALPHA;
            
        std::string str() const
        {
            if (phase != Phase::STABLE) {
                return std::format("{}.{}.{}-{}", major, minor, revision, phaseToString(phase));
            }

            return std::format("{}.{}.{}", major, minor, revision);
        }
    };
}