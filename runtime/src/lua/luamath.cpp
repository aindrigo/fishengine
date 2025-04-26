#include "fish/lua/luamath.hpp"

namespace fish::lua
{
    LuaVec3<true> LuaVertex::getPosition()
    {
        LuaVec3<true> vec(this->vertex.position);
        return vec;
    }
}