#include "fish/world.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "fish/material.hpp"
#include "fish/model.hpp"
#include "fish/transform.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>

namespace fish
{
    World::World()
    {}
    
    entt::registry& World::getRegistry() { return this->registry; }

    void World::update()
    {
        for (const auto& system : this->systems)
            try {
                system->update();
            } catch (std::runtime_error& exception) {
                std::cout << "Error processing system " << system << " update: " << exception.what() << std::endl;
            }
    }

    void World::shutdown()
    {
        for (const auto& system : this->systems)
            try {
                system->shutdown();
            } catch (std::runtime_error& exception) {
                std::cout << "Error processing system " << system << " shutdown: " << exception.what() << std::endl;
            }
    }

    entt::entity World::createPanel()
    {
        auto ent = this->registry.create();
        registry.emplace<Panel>(ent, Panel {});
        auto& material = registry.emplace<Material>(ent, Material("2D_Panel"));
        auto& transform = registry.emplace<Transform2D>(ent, Transform2D {
            .alignment = Transform2D::AlignmentMode::CENTER,
            .position = { 0.0f, 0.0f }
        });
        
        return ent;
    }
}