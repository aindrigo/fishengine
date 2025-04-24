#include "assimp/DefaultLogger.hpp"
#include "assimp/Importer.hpp"
#include "assimp/Logger.hpp"
#include "assimp/cimport.h"
#include "assimp/material.h"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"
#include "assimp/quaternion.h"
#include "assimp/scene.h"
#include "assimp/types.h"
#include "assimp/vector3.h"
#include "fish/assets.hpp"
#include "fish/common.hpp"
#include "fish/helpers.hpp"
#include "fish/material.hpp"
#include "fish/model.hpp"
#include "fish/scenes.hpp"
#include "fish/services.hpp"
#include "fish/texture.hpp"
#include "fish/transform.hpp"
#include "fish/world.hpp"
#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include <format>
#include <stdexcept>
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include <filesystem>
#include <vector>

namespace fish
{
    TextureWrapMode assimpWrapToFishWrap(aiTextureMapMode mapMode)
    {
        TextureWrapMode wrapMode;
        switch (mapMode) {
            case aiTextureMapMode_Clamp:
                wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
                break;
            case aiTextureMapMode_Wrap:
                wrapMode = TextureWrapMode::CLAMP_TO_BORDER; // idk what Wrap means
                break;
            case aiTextureMapMode_Decal:
                wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
                break;
            case aiTextureMapMode_Mirror:
                wrapMode = TextureWrapMode::MIRRORED_REPEAT;
                break;
            default:
                wrapMode = TextureWrapMode::REPEAT;
                break;
        }

        return wrapMode;
    }

    SceneLoader::SceneLoader(Services& services)
        : services(services)
    {}

    SceneLoader::SceneLoader(const SceneLoader& other)
        : services(other.services), importer()
    {}

    void SceneLoader::init()
    {
        Assimp::Logger::LogSeverity severity = Assimp::Logger::NORMAL;
        this->importer.SetIOHandler(new ASIOHandler(services));
        Assimp::DefaultLogger::create("", severity, aiDefaultLogStream_STDOUT);
    }

    Scene SceneLoader::load(const std::filesystem::path& path)
    {
        auto& assets = services.getService<Assets>();
        FISH_ASSERTF(assets.exists(path), "Scene {} not found", path.string());

        const aiScene* scene = importer.ReadFile(
            path.string(),
            aiProcessPreset_TargetRealtime_Fast
        );

        if (scene == nullptr) {
            throw std::runtime_error(std::format("Could not open scene {}!: {}", path.string(), importer.GetErrorString()));
        }
        
        Scene result;
        auto node = scene->mRootNode;
        processNode(node, scene, result);

        return result;
    }

    void SceneLoader::processNode(aiNode* node, const aiScene* assimpScene, Scene& scene)
    {
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            auto child = node->mChildren[i];
            processNode(child, assimpScene, scene);
        }
        
        auto matrix = node->mTransformation;
        {
            aiNode* next = node->mParent;

            while (next != nullptr) {
                matrix = next->mTransformation * matrix;
                next = next->mParent;
            }
        }

        aiVector3D position;
        aiQuaternion rotation;
        aiVector3D scale;

        matrix.Decompose(scale, rotation, position);


        for (int i = 0; i < node->mNumMeshes; i++) {
            auto assimpMeshId = node->mMeshes[i];
            auto assimpMesh = assimpScene->mMeshes[assimpMeshId];

            auto uvList = assimpMesh->mTextureCoords[0];

            Scene::Model model = {
                .vertices = std::vector<Vertex>(assimpMesh->mNumVertices),
                .indices = std::vector<unsigned int>(),
                .position = helpers::Math::toGLM(position),
                .rotation = helpers::Math::toGLM(rotation),
                .scale = helpers::Math::toGLM(scale) * 0.1f
            };

            for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++) {
                auto& position = assimpMesh->mVertices[i];
                auto& normal = assimpMesh->mNormals[i];
                auto& tangent = assimpMesh->mTangents[i];
                auto& bitangent = assimpMesh->mBitangents[i];
                auto& uv = uvList[i];

                model.vertices[i] = Vertex {
                    .position = glm::vec3(position.x, position.y, position.z),
                    .normal = glm::vec3(normal.x, normal.y, normal.z),
                    .tangent = glm::vec3(tangent.x, tangent.y, tangent.z),
                    .bitangent = glm::vec3(bitangent.x, bitangent.y, bitangent.z),
                    .uv = glm::vec2(uv.x, uv.y)
                };

                if (assimpScene->HasMaterials()) {
                    aiMaterial* material = assimpScene->mMaterials[assimpMesh->mMaterialIndex];
                    Scene::Material sceneMat;

                    // diffuse
                    aiString diffusePath;
                    aiTextureMapMode diffuseMapMode;
                    material->GetTexture(aiTextureType_DIFFUSE, 0, &diffusePath, NULL, NULL, NULL, NULL, &diffuseMapMode);
                    if (diffusePath.length > 0) {
                        TextureWrapMode diffuseWrapMode = assimpWrapToFishWrap(diffuseMapMode);
                        sceneMat.diffuseMap = diffusePath.C_Str();
                        sceneMat.diffuseWrapMode = diffuseWrapMode;
                    }

                    // normal
                    if (material->GetTextureCount(aiTextureType_HEIGHT) > 0) {
                        aiString path;
                        aiTextureMapMode mapMode;
                        material->GetTexture(aiTextureType_HEIGHT, 0, &path, NULL, NULL, NULL, NULL, &mapMode);
                        
                        TextureWrapMode normalWrapMode = assimpWrapToFishWrap(mapMode);
                        sceneMat.normalMap = path.C_Str();
                        sceneMat.normalWrapMode = normalWrapMode;
                    }
                    model.material = sceneMat;
                }
            }

            for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++) {
                auto& face = assimpMesh->mFaces[i];
                model.indices.push_back(face.mIndices[0]);
                model.indices.push_back(face.mIndices[1]);
                model.indices.push_back(face.mIndices[2]);
            }

            scene.models.push_back(model);
        }
    }

    void SceneLoader::loadIntoWorld(Scene& scene)
    {
        auto& world = this->services.getService<World>();
        auto& registry = world.getRegistry();

        for (auto const& model : scene.models) {
            auto ent = world.create();
            auto& transform = registry.emplace<Transform3D>(ent);
    
            transform.position = model.position;
            transform.rotation = model.rotation;
            transform.scale = model.scale;
            
            registry.emplace<Mesh>(ent, Mesh {
                .vertices = model.vertices,
                .indices = model.indices
            });
            
            Material material("3D_Lit");
            if (model.material.has_value()) {
                if (!model.material->diffuseMap.empty()) {
                    material.setProperty<std::string>("diffuseMap", model.material->diffuseMap);
                    material.setProperty<TextureWrapMode>("diffuseWrapMode", model.material->diffuseWrapMode);
                }
                if (!model.material->normalMap.empty()) {
                    material.setProperty<std::string>("normalMap", model.material->normalMap);
                    material.setProperty<TextureWrapMode>("normalWrapMode", model.material->normalWrapMode);
                }
            }

            registry.emplace<Material>(ent, material);
        }
    }
}