#pragma once

#include "assimp/IOStream.hpp"
#include "assimp/IOSystem.hpp"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "fish/assets.hpp"
#include "fish/services.hpp"
#include "fish/model.hpp"
#include "fish/texture.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/quaternion.hpp" // IWYU pragma: keep
#include "glm/fwd.hpp"
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace fish
{
    struct Scene {
        struct Material {
            std::string diffuseMap;
            TextureWrapMode diffuseWrapMode;

            std::string normalMap;
            TextureWrapMode normalWrapMode;
        };
        struct Model {
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;

            std::optional<Material> material;
            glm::vec3 position;
            glm::quat rotation;
            glm::vec3 scale { 1, 1, 1 };
        };

        std::unordered_map<int, Material> materials;
        std::vector<Model> models;

        void operator+(const Scene& other) {
            models.insert(models.end(), other.models.begin(), other.models.end());
        }
    };

    class SceneLoader
    {
    public:
        SceneLoader(Services& services);
        SceneLoader(const SceneLoader& other);
        
        void init();
        Scene load(const std::filesystem::path& path);
        void loadIntoWorld(Scene& scene);
    private:
        class ASIOHandler : public Assimp::IOSystem
        {
        public:
            ASIOHandler(Services& services);

            bool Exists(const char* file) const override;
            char getOsSeparator() const override;
            Assimp::IOStream* Open(const char* file, const char* mode) override;
            void Close(Assimp::IOStream* stream) override;
        private:
            class ASIOStream : public Assimp::IOStream
            {
            public:
                ASIOStream(const std::vector<unsigned char>& data);
                size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override;
                size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) override;
                aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override;
                size_t Tell() const override;
                size_t FileSize() const override;
                void Flush() override;
            private:
                size_t cursor = 0;
                std::vector<unsigned char> data;
            };
            Services& services;
            Assets& assets;
        };
        void processNode(aiNode* node, const aiScene* assimpScene, Scene& scene);
        Assimp::Importer importer;
        Services& services;
    };
}