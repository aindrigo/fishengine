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
    {
        this->registry.on_destroy<entt::entity>().connect<&World::onEntityDestroy>(this);
        this->root = registry.create();
        this->registry.emplace<Node>(this->root);
    }
    
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
        if (parent == entt::null) {
            parent = root;
        }
        
        auto& node = registry.get<Node>(entity);
        if (node.parent == parent)
            return;

        auto& parentNode = registry.get<Node>(parent);
        if (node.parent != entt::null) {
            this->clearEntityParent(entity);
        }
        
        moveEntityParent(entity, parent);

        parentNode.children += 1;
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

    void World::clearEntityParent(entt::entity entity)
    {
        FISH_ASSERT(isValid(entity), "Cannot clear parent on invalid entity");
        auto& node = this->registry.get<Node>(entity);
        FISH_ASSERT(isValid(node.parent), "Cannot clear parent on an entity with no parent");

        auto& parentNode = this->registry.get<Node>(node.parent);
        
        if (isValid(node.nextParentChild)) {
            auto& nextParentNode = this->registry.get<Node>(node.nextParentChild);
            if (parentNode.firstChild == entity) {
                parentNode.firstChild = nextParentNode.me;
            }

            if (isValid(node.previousParentChild)) {
                auto& previousParentNode = this->registry.get<Node>(node.previousParentChild);
                previousParentNode.nextParentChild = nextParentNode.me;
                nextParentNode.previousParentChild = previousParentNode.me;
            } else {
                nextParentNode.previousParentChild = entt::null;
            }
        } else if (parentNode.firstChild == entity) {
            parentNode.firstChild = entt::null;
        }

        node.parent = entt::null;
        parentNode.children -= 1;
    }

    void World::moveEntityParent(entt::entity entity, entt::entity parent)
    {
        FISH_ASSERT(isValid(entity), "Cannot clear parent on invalid entity");
        FISH_ASSERT(isValid(parent), "Cannot move entity to an invalid parent");
        auto& node = this->registry.get<Node>(entity);
        auto& parentNode = registry.get<Node>(parent);
    
        if (!isValid(parentNode.firstChild)) {
            parentNode.firstChild = entity;
            return;   
        }

        entt::entity next = parentNode.firstChild;

        while (isValid(next)) {            
            Node& nextNode = registry.get<Node>(next);
            if (nextNode.nextParentChild == entt::null) {
                nextNode.nextParentChild = entity;
                node.previousParentChild = next;
                break;
            }

            next = nextNode.nextParentChild;
        }
    }

    void World::onEntityDestroy(entt::entity entity)
    {
        if (!isValid(entity))
            return;

        auto& node = this->registry.get<Node>(entity);
        if (isValid(node.parent)) {
            this->clearEntityParent(entity);
        }

        if (node.children > 1 && isValid(node.firstChild)) {
            auto& next = node.firstChild;

            while (isValid(next)) {
                auto& nextNode = this->registry.get<Node>(next);
                next = nextNode.nextParentChild; 

                clearEntityParent(nextNode.me);
                moveEntityParent(nextNode.me, root);
            }
        }
    }
}