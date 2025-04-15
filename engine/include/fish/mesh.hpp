#pragma once

#include <vector>
namespace fish
{
    struct Mesh
    {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
    };

    struct MeshGPUData {
        unsigned int ebo;
        unsigned int vbo;
        unsigned int vao;
    };
}