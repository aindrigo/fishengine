#pragma once

#include "fish/events.hpp"
#include <sol/optional_implementation.hpp>
#include <string>
namespace fish::lua
{
    struct LuaEventData {
        LuaEventData(EventData& data);

        sol::optional<std::string> getString(std::string name);
    private:
        EventData& data;
    };
}