#include "assimp/DefaultLogger.hpp"
#include "assimp/Importer.hpp"
#include "assimp/Logger.hpp"
#include "assimp/cimport.h"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/types.h"
#include "fish/assets.hpp"
#include "fish/helpers.hpp"
#include "fish/model.hpp"
#include "fish/scenes.hpp"
#include "fish/services.hpp"
#include "fish/world.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/gtc/quaternion.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"
#include <filesystem>
#include <vector>

namespace fish
{
    SceneLoader::SceneLoader(Services& services)
        : services(services)
    {}

    SceneLoader::SceneLoader(const SceneLoader& other)
        : services(other.services), importer()
    {}

    void SceneLoader::init()
    {
        Assimp::Logger::LogSeverity severity = Assimp::Logger::NORMAL;

        Assimp::DefaultLogger::create("", severity, aiDefaultLogStream_STDOUT);
    }

    Scene SceneLoader::load(const std::filesystem::path& path)
    {
        auto& assets = services.getService<Assets>();
        FISH_ASSERTF(assets.exists(path), "Scene {} not found", path.string());

        std::vector<unsigned char> data = assets.findAssetBytes(path);

        const aiScene* scene = importer.ReadFileFromMemory(
            data.data(),
            data.size(),
            aiProcessPreset_TargetRealtime_Fast,
            ""
        );
        
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

        glm::mat4 matrix = helpers::Math::toGLM(node->mTransformation);
        {
            aiNode* next = node->mParent;

            while (next != nullptr) {
                matrix *= helpers::Math::toGLM(next->mTransformation);
                next = next->mParent;
            }
        }

        glm::vec3 position;
        glm::quat orientation;
        glm::vec3 scale;
        glm::vec3 skew;
        glm::vec4 persp;
        glm::decompose(matrix, scale, orientation, position, skew, persp);


        for (int i = 0; i < node->mNumMeshes; i++) {

            auto assimpMeshId = node->mMeshes[i];
            auto assimpMesh = assimpScene->mMeshes[assimpMeshId];

            auto uvList = assimpMesh->mTextureCoords[0];

            Scene::Model model = {
                .vertices = std::vector<Vertex>(assimpMesh->mNumVertices),
                .indices = std::vector<unsigned int>(),
                .position = position,
                .eulerAngles = glm::eulerAngles(orientation),
                .scale = scale
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
            transform.eulerAngles = model.eulerAngles;
            transform.scale = model.scale;
            
            registry.emplace<Mesh>(ent, Mesh {
                .vertices = model.vertices,
                .indices = model.indices
            });
            registry.emplace<Material>(ent, Material("3D_Lit"));
        }
    }
}