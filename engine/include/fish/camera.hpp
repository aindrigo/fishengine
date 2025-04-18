#pragma once

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/trigonometric.hpp"
#include "glm/vec3.hpp" // IWYU pragma: keep // dunno why I have to do this, thanks clangd


namespace fish
{
    struct Camera3D {
        float fov = 60.0f;
        float zNear = 0.01f;
        float zFar = 15000.0f;

        glm::mat4 build(float aspectRatio) 
        {
            return glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar);
        }
    };
}