#pragma once

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include <vector>
namespace fish
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
        glm::vec2 uv;
    };

    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };

    struct Panel
    {
        bool movable = false;
    };

    struct VertexGPUData {
        unsigned int ebo;
        unsigned int vbo;
        unsigned int vao;
    };
}