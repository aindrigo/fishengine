#include "fish/world.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "fish/material.hpp"
#include "fish/model.hpp"
#include "fish/transform.hpp"
#include "fish/node.hpp"
#include "fish/common.hpp"
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

    bool World::isValid(entt::entity entity)
    {
        return this->registry.valid(entity) && this->registry.any_of<Node>(entity);
    }

    void World::setParent(entt::entity entity, entt::entity parent)
    {
        FISH_ASSERT(isValid(entity), "Entity to set parent on must be valid");
        FISH_ASSERT(isValid(parent) || parent == entt::null, "Node parent must have a Node component or be null");
        
        auto& entityNode = registry.get<Node>(entity);

        if (isValid(entityNode.nextParentChild)) {
            Node& nextParentNode = registry.get<Node>(entityNode.nextParentChild);
            if (isValid(entityNode.previousParentChild)) {
                Node& previousParentNode = registry.get<Node>(entityNode.previousParentChild);
                previousParentNode.nextParentChild = nextParentNode.me;
                nextParentNode.previousParentChild = previousParentNode.me;
            }
        }

        if (entityNode.parent != entt::null) {
            auto& parentNode = registry.get<Node>(entityNode.parent);
            parentNode.children -= 1;
        }
        
        if (parent == entt::null) {
            entityNode.parent = entt::null;
            return;
        }

        auto& newParentNode = registry.get<Node>(parent);
    
        if (!isValid(newParentNode.firstChild)) {
            newParentNode.firstChild = entity;
            return;   
        }

        entt::entity next = newParentNode.firstChild;

        while (isValid(next)) {            
            Node& nextNode = registry.get<Node>(next);
            if (nextNode.nextParentChild == entt::null) {
                nextNode.nextParentChild = entity;
                entityNode.previousParentChild = next;
                break;
            }

            next = nextNode.nextParentChild;
        }
    }

    entt::entity World::create(entt::entity parent)
    {
        entt::entity newEntity = this->registry.create();
        
        auto& node = registry.emplace<Node>(newEntity);
        setParent(newEntity, parent);
        return newEntity;
    }

    entt::entity World::createPanel()
    {
        auto ent = this->create();
        registry.emplace<Panel>(ent, Panel {});
        auto& material = registry.emplace<Material>(ent, Material("2D_Panel"));
        auto& transform = registry.emplace<Transform2D>(ent, Transform2D {
            .alignment = Transform2D::AlignmentMode::CENTER,
            .position = { 0.0f, 0.0f }
        });
        
        return ent;
    }
}