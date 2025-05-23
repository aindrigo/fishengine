#include "fish/world.hpp"
#include "entt/entity/entity.hpp"
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
        : camera(entt::null)
    {
        this->registry.on_destroy<entt::entity>().connect<&World::onEntityDestroy>(this);
        this->root = registry.create();
        this->registry.emplace<Node>(this->root, Node { .me = this->root });
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

    void World::preRender()
    {
        for (const auto& system : this->systems)
            try {
                system->preRender();
            } catch (std::runtime_error& exception) {
                std::cout << "Error processing system " << system << " prerender: " << exception.what() << std::endl;
            }
    }

    void World::render()
    {
        for (const auto& system : this->systems)
            try {
                system->render();
            } catch (std::runtime_error& exception) {
                std::cout << "Error processing system " << system << " render: " << exception.what() << std::endl;
            }
    }
    
    void World::postRender()
    {
        for (const auto& system : this->systems)
            try {
                system->postRender();
            } catch (std::runtime_error& exception) {
                std::cout << "Error processing system " << system << " postrender: " << exception.what() << std::endl;
            }
    }
    
    void World::tick()
    {
        for (const auto& system : this->systems)
            try {
                system->tick();
            } catch (std::runtime_error& exception) {
                std::cout << "Error processing system " << system << " tick: " << exception.what() << std::endl;
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

    entt::entity World::getCamera()
    {
        return this->camera;
    }

    void World::setCamera(entt::entity entity)
    {
        this->camera = entity;
    }

    bool World::isValid(entt::entity entity)
    {
        return this->registry.valid(entity) && this->registry.any_of<Node>(entity);
    }

    entt::entity World::getParent(entt::entity entity)
    {
        FISH_ASSERT(isValid(entity), "Entity to get parent on must be valid");

        auto& node = registry.get<Node>(entity);
        return node.parent;
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
        
        auto& node = registry.emplace<Node>(newEntity, Node { .me = newEntity });
        setParent(newEntity, parent);
        return newEntity;
    }

    entt::entity World::createPanel()
    {
        auto ent = this->create();
        registry.emplace<Panel>(ent, Panel {});
        auto& material = registry.emplace<Material>(ent, Material("2D_Panel"));
        auto& transform = registry.emplace<Transform2D>(ent, Transform2D {
            .alignment = Transform2D::AlignmentMode::TOP_LEFT,
            .position = { 0.0f, 0.0f }
        });
        
        return ent;
    }

    void World::destroy(entt::entity entity)
    {
        FISH_ASSERT(isValid(entity), "Cannot destroy an invalid entity");
        auto& node = this->registry.get<Node>(entity);
        {
            auto& next = node.firstChild;
            while (isValid(next)) {
                clearEntityParent(next);
            }
        }

        clearEntityParent(entity);
        registry.destroy(entity);
    }

    Transform3D World::worldSpace3DTransform(entt::entity entity)
    {
        FISH_ASSERT(isValid(entity), "Cannot build transform for an invalid entity");
        FISH_ASSERT(registry.all_of<Transform3D>(entity), "Cannot build 3D transform on an entity with no Transform3D component");

        auto& transform = registry.get<Transform3D>(entity);

        auto& node = registry.get<Node>(entity);
        
        Transform3D result = transform;
        if (!isValid(node.parent))
            return result;

        entt::entity next = node.parent;
        while (isValid(next)) {
            auto& nextNode = registry.get<Node>(next);
            next = nextNode.parent;
            if (!registry.all_of<Transform3D>(nextNode.me))
                break;

            auto& transform = registry.get<Transform3D>(nextNode.me);
            result.position += transform.position;
            result.rotation *= transform.rotation;
            result.scale *= transform.scale;
        }

        return result;
    }

    Transform2D World::worldSpace2DTransform(entt::entity entity)
    {
        FISH_ASSERT(isValid(entity), "Cannot build transform for an invalid entity");
        FISH_ASSERT(registry.all_of<Transform2D>(entity), "Cannot build 2D transform on an entity with no Transform2D component");

        auto& transform = registry.get<Transform2D>(entity);
        Transform2D result = transform;

        auto& node = registry.get<Node>(entity);
        
        if (!isValid(node.parent))
            return result;

        entt::entity next = node.parent;
        while (isValid(next)) {
            auto& nextNode = registry.get<Node>(next);
            next = nextNode.parent;
            if (!registry.all_of<Transform2D>(nextNode.me))
                break;

            auto& transform = registry.get<Transform2D>(nextNode.me);
            result.position += transform.position;
            //result.size *= transform.size;
        }

        return result;
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
    
        node.parent = parent;

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