#pragma once

#include <optional>
#include <memory>
#include <string>
#include <unordered_map>

namespace fish
{
    class IPropertyHolder
    {
    public:
        template <typename T>
        void setProperty(const std::string& name, T data)
        {
            this->properties[name] = std::make_shared<PropertyContainer<T>>(data);
        }

        bool hasProperty(const std::string& name)
        {
            return this->properties.contains(name);
        }

        template <typename T>
        std::optional<T> getProperty(const std::string& name)
        {
            if (!this->hasProperty(name))
                return std::nullopt;
            if (auto container = dynamic_cast<PropertyContainer<T>*>(this->properties.at(name).get()))
                return container->data;
            return std::nullopt;
        }
    private:
        struct IPropertyContainer {
            virtual ~IPropertyContainer() = default;
        };

        template <typename T>
        struct PropertyContainer : public IPropertyContainer {
            PropertyContainer<T>(T data)
                : data(data)
            {}

            T data;
        };
        std::unordered_map<std::string, std::shared_ptr<IPropertyContainer>> properties;
    };
}