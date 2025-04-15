#pragma once

#include "glm/vec3.hpp" // IWYU pragma: keep // dunno why I have to do this, thanks clangd

namespace fish
{
    struct Camera3D {
        glm::vec3 eulerAngles;
        glm::vec3 position;
    };
}