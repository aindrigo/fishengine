#include "fish/world.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>

namespace fish
{
    World::World()
        : registry(std::make_shared<entt::registry>())
    {}
    
    std::weak_ptr<entt::registry> World::getRegistry()
    {
        return this->registry;
    }

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
}