#pragma once

#include "glm/ext/vector_float3.hpp"
#include <vector>
namespace fish
{
    struct Vertex
    {
        glm::vec3 position;
    };

    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };

    struct Panel
    {
    };

    struct VertexGPUData {
        unsigned int ebo;
        unsigned int vbo;
        unsigned int vao;
    };
}