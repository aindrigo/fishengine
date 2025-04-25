#pragma once

#include "fish/common.hpp"
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace fish
{
    class Services
    {
    public:
        Services() = default;

        template <typename T>
        bool exists()
        {
            return this->services.contains(typeid(T));
        }

        template <typename T>
        T& addServiceData(const T& data)
        {
            FISH_ASSERTF(!this->exists<T>(), "Service {} already exists", typeid(T).name());
            
            this->services[typeid(T)] = std::make_shared<ServiceContainer<T>>(data);
            return dynamic_cast<ServiceContainer<T>*>(this->services.at(typeid(T)).get())->data;
        }

        template <typename T, typename ...Args>
        T& addService(const Args... args)
        {
            FISH_ASSERTF(!this->exists<T>(), "Service {} already exists", typeid(T).name());
            
            this->services[typeid(T)] = std::make_shared<ServiceContainer<T>>(args...);
            return dynamic_cast<ServiceContainer<T>*>(this->services.at(typeid(T)).get())->data;
        }

        template <typename T>
        T& getService()
        {
            FISH_ASSERTF(this->exists<T>(), "Service {} does not exist", typeid(T).name());
            
            ServiceContainer<T>* servicePtr = dynamic_cast<ServiceContainer<T>*>(this->services.at(typeid(T)).get());
            return servicePtr->data;
        }
    private:
        struct IServiceContainer {
            virtual ~IServiceContainer() = default;
        };

        template <typename T>
        struct ServiceContainer : public IServiceContainer {
            ServiceContainer(const T& data)
                : data(data)
            {}

            template <typename ...Args>
            ServiceContainer(const Args... args)
                : data(T(args...))
            {}
            T data;
        };

        std::unordered_map<std::type_index, std::shared_ptr<IServiceContainer>> services;
    };
}