#pragma once

#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/quaternion.hpp"
#include <utility>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"

namespace fish
{
    
    struct Transform3D {
        glm::vec3 position;
        glm::vec3 pivotPoint;
        glm::quat rotation;
        glm::vec3 scale { 1, 1, 1 };

        glm::vec3 forward()
        {
            return glm::vec3(
                (rotation * glm::vec3(-1.0f, 0.0f, 0.0f)).z,
                0,
                (rotation * glm::vec3(0.0f, 0.0f, 1.0f)).z
            );
        }

        glm::vec3 right()
        {
            return glm::vec3(
                (rotation * glm::vec3(0.0f, 0.0f, 1.0f)).z,
                0,
                (rotation * glm::vec3(1.0f, 0.0f, 0.0f)).z
            );
        }

        glm::mat4 build() 
        {
            auto pivotFrom = glm::translate(glm::mat4(1.0), pivotPoint + position);
            auto pivotTo = glm::translate(glm::mat4(1.0), -(pivotPoint + position));

            auto scalingMatrix = glm::scale(glm::mat4(1.0), scale);
            auto rotationMatrix = glm::mat4_cast(rotation);
            auto translationMatrix = glm::translate(glm::mat4(1.0), position);
            
            return pivotFrom * rotationMatrix * scalingMatrix * pivotTo * translationMatrix;
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
                    return { position, position + size };
                default:
                    return { glm::vec2(0.0f), glm::vec2(0.0f) };
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