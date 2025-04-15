#pragma once

#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"

namespace fish
{
    struct Transform {
        glm::vec3 position;
        glm::vec3 eulerAngles;
        glm::vec3 scale { 1, 1, 1 };

        glm::mat4 build() 
        {
            glm::mat4 matrix = glm::identity<glm::mat4>();
            matrix = glm::scale(matrix, scale);
            matrix *= glm::eulerAngleXYZ(eulerAngles.x, eulerAngles.y, eulerAngles.z);
            matrix = glm::translate(matrix, position);
            return matrix;
        }
    };
}