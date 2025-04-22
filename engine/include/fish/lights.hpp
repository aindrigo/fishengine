#pragma once

#include "fish/material.hpp"
#include "fish/transform.hpp"
#include "glm/ext/vector_float4.hpp"
namespace fish
{
    struct PointLight {
        float constant = 0.1f;
        float linear = 0.2f;
        float quadratic = 0.5f;
        Color color;

        struct alignas(16) GLSL {
            glm::vec4 position;
            glm::vec4 color;
            glm::vec4 clq;
        };
        
        GLSL toGL(Transform3D worldSpace)
        {
            return {
                .position = glm::vec4(worldSpace.position, 0.0f),
                .color = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f },
                .clq = glm::vec4(constant, linear, quadratic, 0.0f)
            };
        }
    };
}