#pragma once

#include "fish/system.hpp"
namespace fish
{
    class RuntimeSystem : public ISystem
    {
    public:
        void update() override;
    };
}