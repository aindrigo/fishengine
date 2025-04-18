#pragma once

#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include <utility>
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
            glm::mat4 matrix = glm::mat4(1.0f);
            matrix = glm::translate(matrix, position);
            matrix *= glm::eulerAngleXYZ(eulerAngles.x, eulerAngles.y, eulerAngles.z);
            matrix = glm::scale(matrix, scale);
            return matrix;
        }
    };

    struct Transform2D {
        enum struct AlignmentMode {
            TOP_LEFT,
            CENTER
        };

        AlignmentMode alignment;
        int z = 0;
        glm::vec2 position;
        glm::vec2 size = { 128.0f, 128.0f };

        std::pair<glm::vec2, glm::vec2> bounds()
        {
            switch (alignment) {
                case AlignmentMode::CENTER:
                    return { position - size * 0.5f, position + size * 0.5f };
                case AlignmentMode::TOP_LEFT:
                    return {position, position + size};
            }
        }
        
        glm::mat4 build()
        {
            glm::mat4 matrix = glm::mat4(1.0f);
            matrix = glm::translate(matrix, glm::vec3(position, 0.0f));
            switch (alignment) {
                case AlignmentMode::CENTER:
                    break;
                case AlignmentMode::TOP_LEFT:
                    matrix = glm::translate(matrix, glm::vec3(size * 0.5f, 0.0f));
                    break;
            }

            matrix = glm::scale(matrix, glm::vec3(size, 1.0f));


            return matrix;
        }
    };
}