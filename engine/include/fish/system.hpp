#pragma once

#include "entt/entity/fwd.hpp"
#include <memory>
namespace fish
{
    class ISystem
    {
    public:
        virtual ~ISystem() = default;

        virtual void init(std::weak_ptr<entt::registry> registry) = 0;
        virtual void update() = 0;
        virtual void shutdown() = 0;
    };
}