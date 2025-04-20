#pragma once

#include "fish/helpers.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/trigonometric.hpp"
#include "glm/vec3.hpp" // IWYU pragma: keep // dunno why I have to do this, thanks clangd


namespace fish
{
    struct Camera3D {
        float fov = 60.0f;
        float zNear = 0.01f;

        glm::mat4 build(float aspectRatio) 
        {
            return helpers::Math::perspectiveReverseZ(glm::radians(fov), aspectRatio, zNear);
        }
    };
}