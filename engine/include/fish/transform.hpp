#pragma once

#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"

namespace fish
{
    
    struct Transform3D {
        glm::vec3 position;
        glm::vec3 eulerAngles;
        glm::vec3 scale { 1, 1, 1 };

        glm::mat4 build() 
        {
            glm::mat4 matrix = glm::identity<glm::mat4>();
            matrix = glm::translate(matrix, position);
            matrix *= glm::eulerAngleXYZ(eulerAngles.x, eulerAngles.y, eulerAngles.z);
            matrix = glm::scale(matrix, scale);
            return matrix;
        }
    };

    struct Transform2D {
        enum struct AlignmentMode {
            CENTER,
            TOP_LEFT
        };

        AlignmentMode alignment;
        int z = 0;
        glm::vec2 position;
        glm::vec2 size = { 128.0f, 128.0f };

        glm::mat4 build()
        {
            glm::mat4 matrix = glm::mat4(1.0f);
            matrix = glm::translate(matrix, glm::vec3(position, 0.0f));
            matrix = glm::scale(matrix, glm::vec3(size, 1.0f));

            switch (alignment) {
                case AlignmentMode::CENTER:
                    //matrix = glm::translate(matrix, glm::vec3(-size.x / 2.0f, -size.y / 2.0f, 0.0f));
                    break;
                case AlignmentMode::TOP_LEFT:
                    break;
            }

            return matrix;
        }
    };
}