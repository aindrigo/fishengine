#include "assimp/DefaultLogger.hpp"
#include "assimp/Importer.hpp"
#include "assimp/Logger.hpp"
#include "assimp/cimport.h"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/types.h"
#include "fish/assets.hpp"
#include "fish/scenes.hpp"
#include "fish/services.hpp"
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

        for (int i = 0; i < scene->mNumMeshes; i++) {
            const aiMesh* assimpMesh = scene->mMeshes[i];
            
        }

        return result;
    }
}