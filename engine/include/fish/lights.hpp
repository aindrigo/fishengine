#pragma once

#include "fish/material.hpp"
#include "fish/transform.hpp"
#include "glm/ext/vector_float3.hpp"

namespace fish
{
    struct PointLight {
        float constant = 1.0f;
        float linear = 0.09f;
        float quadratic = 0.032f;
        float intensity = 2.0f;
        Color color;

        struct alignas(16) GLSL {
            alignas(16) glm::vec4 position;
            alignas(16) glm::vec4 color;
            alignas(16) glm::vec4 clq;
            alignas(16) int enabled;
        };
        
        GLSL toGL(Transform3D worldSpace)
        {
            return {
                .position = { worldSpace.position.x, worldSpace.position.y, worldSpace.position.z, 0.0f },
                .color = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f },
                .clq = { constant, linear, quadratic, intensity },
                .enabled = 1
            };
        }
    };

    struct DirectionalLight {
        glm::vec3 direction;
        Color color;
    };
}