#pragma once

#include "assimp/Importer.hpp"
#include "fish/services.hpp"
#include "fish/model.hpp"
#include "glm/ext/vector_float3.hpp"
#include <filesystem>
#include <vector>
namespace fish
{
    struct Scene {
        struct Model {
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;

            glm::vec3 position;
            glm::vec3 eulerAngles;
            glm::vec3 scale { 1, 1, 1 };
        };
        std::vector<Model> models;
    };

    class SceneLoader
    {
    public:
        SceneLoader(Services& services);
        SceneLoader(const SceneLoader& other);
        
        void init();
        Scene load(const std::filesystem::path& path);
    private:
        Assimp::Importer importer;
        Services& services;
    };
}