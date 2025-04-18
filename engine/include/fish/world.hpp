#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "fish/system.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "system.hpp"
#include <memory>
#include <type_traits>
#include <vector>

namespace fish
{
    class World
    {
    public:
        World();
        entt::registry& getRegistry();

        template <typename SystemType, typename... Args>
        void addSystem(const Args... args)
        {
            static_assert(std::is_base_of<ISystem, SystemType>(), "Systems must be derived from ISystem");
            
            std::unique_ptr<ISystem> system = std::make_unique<SystemType>(args...);
            system->init();
            systems.emplace_back(std::move(system));
            
            return;
        }
    
        bool isValid(entt::entity entity);
        void setParent(entt::entity entity, entt::entity parent = entt::null);

        entt::entity create(entt::entity parent = entt::null);
        entt::entity createPanel();
        
        glm::mat4 build3DTransform(entt::entity entity);
        glm::mat4 build2DTransform(entt::entity entity);

        void update();
        void shutdown();
    
    private:
        entt::entity root;
        void clearEntityParent(entt::entity entity);
        void moveEntityParent(entt::entity entity, entt::entity parent);

        void onEntityDestroy(entt::entity entity);
        std::vector<std::unique_ptr<ISystem>> systems;
        entt::registry registry;
    };
}