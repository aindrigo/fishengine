#include "fish/lua/luaevents.hpp"
#include "fish/events.hpp"
#include <sol/optional_implementation.hpp>
#include <string>

namespace fish::lua
{
    LuaEventData::LuaEventData(EventData& data)
        : data(data)
    {}
    
    sol::optional<std::string> LuaEventData::getString(std::string name) 
    {
        auto propertyOpt = this->data.getProperty<std::string>(name);
        if (!propertyOpt.has_value())
            return sol::nullopt;

        return propertyOpt.value();
    }
}