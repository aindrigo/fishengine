#pragma once

#include "fish/engine.hpp"
#include "fish/version.hpp"
#include <memory>
#include <set>
#include <string>

namespace fish
{
    class Runtime
    {
    public:
        Runtime(std::set<std::string> arguments);

        void run();
    private:
        std::string name;
        Version version;
        
        std::unique_ptr<Engine> engine;
        void loadGameData();
    };
}