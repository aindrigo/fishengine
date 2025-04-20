#pragma once

#include "fish/assets.hpp"
#include "fish/engineinfo.hpp"
#include "fish/services.hpp"
#include <cstddef>
#include <filesystem>
#include <unordered_map>
namespace fish
{
    struct Texture2DData {
        float expiryTime;
        unsigned int textureId;
        int width;
        int height;
        int channels;
    };

    enum struct TextureFilterMode {
        NEAREST,
        LINEAR
    };

    enum struct TextureWrapMode {
        CLAMP_TO_EDGE,
        REPEAT,
        MIRRORED_REPEAT,
        CLAMP_TO_BORDER,
        MIRROR_CLAMP_TO_EDGE,
    };

    struct TextureFileData {
        std::string path;
        TextureFilterMode filter;
        TextureWrapMode wrapMode;

        bool operator==(const TextureFileData& other) const
        {
            return other.path == this->path &&
                   other.filter == this->filter &&
                   other.wrapMode == this->wrapMode;
        }
    };
    
    struct TextureFileDataHasher
    {
        std::size_t operator()(const fish::TextureFileData& f) const
        {
            return ((std::hash<std::string>()(f.path)
                    ^ (std::hash<fish::TextureFilterMode>()(f.filter) << 1)) >> 1)
                    ^ (std::hash<fish::TextureWrapMode>()(f.wrapMode) << 1);
        }
    };

    class TextureManager
    {
    public:
        TextureManager(Services& services);
        unsigned int get(const std::filesystem::path& path, 
            TextureFilterMode filterMode = TextureFilterMode::LINEAR,
            TextureWrapMode wrapMode = TextureWrapMode::CLAMP_TO_EDGE
        );
        void bind(const std::filesystem::path& path, unsigned int bindingPoint);
        void bind(unsigned int id, unsigned int bindingPoint);
        void gc();
    private:
        

        Texture2DData loadTextureFromFile(TextureFileData fileData);
        
        static constexpr int TEXTURE_EXPIRY_TIME = 15; // any texture that is unused for more than x seconds is deleted
        EngineInfo& engineInfo;
        Assets& assets;
        Services& services;
        std::unordered_map<TextureFileData, Texture2DData, TextureFileDataHasher> textures;
    };
}

