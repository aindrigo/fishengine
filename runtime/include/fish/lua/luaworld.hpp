#pragma once

#include "entt/entity/fwd.hpp"
#include "fish/world.hpp"
#include <sol/optional_implementation.hpp>
namespace fish::lua
{
    class LuaEntityWrapper
    {
    public:
        LuaEntityWrapper(entt::entity entity, World& world);

        entt::entity getEntity();
        bool isValid();
        void destroy();
        LuaEntityWrapper getParent();
    private:
        World& world;
        entt::entity entity;
    };

    class LuaWorld
    {
    public:
        LuaWorld(World& world);
        LuaEntityWrapper create(sol::optional<LuaEntityWrapper>);

        void setCamera(LuaEntityWrapper entity);
        LuaEntityWrapper getCamera();
    private:
        World& world;
    };
}