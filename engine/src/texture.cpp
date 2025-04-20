#include "fish/texture.hpp"
#include "fish/assets.hpp"
#include "fish/engineinfo.hpp"
#include "glad/gl.h"
#include "fish/services.hpp"
#include "glm/common.hpp"
#include "glm/exponential.hpp"
#include "glm/gtc/integer.hpp" // IWYU pragma: keep
#include "stb_image.h"
#include <filesystem>
#include <format>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace fish
{
    TextureManager::TextureManager(Services& services)
        : services(services), engineInfo(services.getService<EngineInfo>()), assets(services.getService<Assets>())
    {
        stbi_set_flip_vertically_on_load(true);
    }

    unsigned int TextureManager::get(const std::filesystem::path& path, TextureFilterMode filterMode, TextureWrapMode wrapMode)
    {
        TextureFileData fileData = {
            .path = path.string(),
            .filter = filterMode,
            .wrapMode = wrapMode
        };

        if (this->textures.contains(fileData)) {
            this->textures[fileData].expiryTime = engineInfo.gameTime + TEXTURE_EXPIRY_TIME;
            return this->textures[fileData].textureId;
        }

        Texture2DData texData = this->loadTextureFromFile(fileData);
        texData.expiryTime = engineInfo.gameTime + TEXTURE_EXPIRY_TIME;
        this->textures[fileData] = texData;
        return texData.textureId;
    }

    void TextureManager::bind(const std::filesystem::path& path, unsigned int bindingPoint)
    {
        unsigned int id = this->get(path);
        glBindTextureUnit(bindingPoint, id);
    }

    void TextureManager::bind(unsigned int id, unsigned int bindingPoint)
    {
        glBindTextureUnit(bindingPoint, id);
    }

    void TextureManager::gc()
    {
        for (auto& [ path, data ] : this->textures)
            if (data.expiryTime < engineInfo.gameTime) {
                glDeleteTextures(1, &data.textureId);
                this->textures.erase(path);
            }
    }

    Texture2DData TextureManager::loadTextureFromFile(TextureFileData fileData)
    {
        std::vector<unsigned char> data = assets.findAssetBytes(fileData.path);
        
        Texture2DData textureData {};
        
        unsigned char* pixels = stbi_load_from_memory(
            data.data(),
            data.size() * sizeof(unsigned char),
            &textureData.width,
            &textureData.height,
            &textureData.channels,
            0
        );
        glCreateTextures(GL_TEXTURE_2D, 1, &textureData.textureId);
        
        GLenum minFilter;
        GLenum magFilter;
        switch (fileData.filter) {
            case TextureFilterMode::LINEAR:
                minFilter = GL_LINEAR_MIPMAP_LINEAR;
                magFilter = GL_LINEAR;
                break;
            case TextureFilterMode::NEAREST:
                minFilter = GL_NEAREST_MIPMAP_LINEAR;
                magFilter = GL_NEAREST;
                break;
        }

        glTextureParameteri(textureData.textureId, GL_TEXTURE_MIN_FILTER, minFilter);
        glTextureParameteri(textureData.textureId, GL_TEXTURE_MAG_FILTER, magFilter);

        GLenum wrapMode;
        switch (fileData.wrapMode) {
            case TextureWrapMode::CLAMP_TO_EDGE:
                wrapMode = GL_CLAMP_TO_EDGE;
                break;
            case TextureWrapMode::REPEAT:
                wrapMode = GL_REPEAT;
                break;
            case TextureWrapMode::MIRRORED_REPEAT:
                wrapMode = GL_MIRRORED_REPEAT;
                break;
            case TextureWrapMode::CLAMP_TO_BORDER:
                wrapMode = GL_CLAMP_TO_BORDER;
                break;
            case TextureWrapMode::MIRROR_CLAMP_TO_EDGE:
                wrapMode = GL_MIRROR_CLAMP_TO_EDGE;
                break;
        }

        std::cout << wrapMode << std::endl;
        glTextureParameteri(textureData.textureId, GL_TEXTURE_WRAP_S, wrapMode);
        glTextureParameteri(textureData.textureId, GL_TEXTURE_WRAP_T, wrapMode);

        GLenum internalFormat;
        GLenum format;

        switch (textureData.channels) {
            case 4:
                internalFormat = GL_RGBA8;
                format = GL_RGBA;
                break;
            case 3:
                internalFormat = GL_RGB8;
                format = GL_RGB;
                break;
            case 2:
                internalFormat = GL_RG8;
                format = GL_RG;
                break;
            case 1:
                internalFormat = GL_R8;
                format = GL_R;
                break;
            default:
                throw std::runtime_error(
                    std::format("Number of channels is unknown for image {}: {}", fileData.path, textureData.channels)
                );
        }

        glTextureStorage2D(
            textureData.textureId,
            1 + glm::floor(glm::log2(glm::max(textureData.width, textureData.height))),
            internalFormat,
            textureData.width,
            textureData.height
        );

        glTextureSubImage2D(
            textureData.textureId, 
            0, 
            0, 
            0, 
            textureData.width, 
            textureData.height, 
            format, 
            GL_UNSIGNED_BYTE, 
            pixels
        );
        
        glGenerateTextureMipmap(textureData.textureId);

        stbi_image_free(pixels);

        return textureData;
    }

}