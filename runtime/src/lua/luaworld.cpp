
#include "fish/lua/luaworld.hpp"
#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"
#include "fish/node.hpp"
#include "fish/world.hpp"
#include <sol/optional_implementation.hpp>
namespace fish::lua
{
    // Entity
    LuaEntityWrapper::LuaEntityWrapper(entt::entity entity, World& world)
        : entity(entity), world(world)
    {}

    entt::entity LuaEntityWrapper::getEntity()
    {
        return this->entity;
    }

    bool LuaEntityWrapper::isValid()
    {
        return this->world.isValid(this->entity);
    }

    void LuaEntityWrapper::destroy()
    {
        if (!this->isValid())
            return;
        
        world.destroy(entity);
    }
    
    LuaEntityWrapper LuaEntityWrapper::getParent()
    {
        if (!this->isValid())
            return { entt::null, this->world };

        auto& node = this->world.getRegistry().get<Node>(this->entity);
        return { node.parent, world };
    }

    // World
    LuaWorld::LuaWorld(World& world)
        : world(world)
    {}

    LuaEntityWrapper LuaWorld::create(sol::optional<LuaEntityWrapper> parent)
    {
        entt::entity parentEntity = parent ? parent.value().getEntity() : entt::null;
        return { this->world.create(parentEntity), world };
    }
    
    void LuaWorld::setCamera(LuaEntityWrapper entity)
    {
        this->world.setCamera(entity.getEntity());
    }

    LuaEntityWrapper LuaWorld::getCamera()
    {
        return { this->world.getCamera(), world };
    }
}