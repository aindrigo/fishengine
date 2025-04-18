#pragma once


#include "fish/property.hpp"
#include <unordered_map>
namespace fish
{
    struct Color {
        unsigned char r = 255;
        unsigned char g = 255;
        unsigned char b = 255;
        unsigned char a = 255;
    };

    class Material : public IPropertyHolder
    {
    public:
        Material(std::string shader)
            : shader(shader)
        {}

        std::string shader;
    private:
    };
}