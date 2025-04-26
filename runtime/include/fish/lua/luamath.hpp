#pragma once

#include "fish/model.hpp"
#include "glm/ext/vector_float3.hpp"
#include <type_traits>
namespace fish::lua
{
    template <bool IsReference = false>
    struct LuaVec3 {
        using Vec3Type = std::conditional_t<IsReference, glm::vec3&, glm::vec3>;

        Vec3Type vec;
        float getX() { return this->vec.x; };
        float getY() { return this->vec.y; };
        float getZ() { return this->vec.z; };
        void setX(float value) { this->vec.x = value; };
        void setY(float value) { this->vec.y = value; };
        void setZ(float value) { this->vec.z = value; };
    };

    struct LuaVertex {
        Vertex& vertex;
        LuaVec3<true> getPosition();
    };
}