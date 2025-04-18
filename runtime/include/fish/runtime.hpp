#pragma once

#include "fish/engine.hpp"
#include "fish/version.hpp"
#include <memory>
#include <string>
namespace fish
{
    class Runtime
    {
    public:
        Runtime();

        void run();
    private:
        std::string name;
        Version version;
        
        std::unique_ptr<Engine> engine;
        void loadGameData();
    };
}