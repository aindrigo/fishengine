#include "assimp/IOStream.hpp"
#include "assimp/types.h"
#include "fish/assets.hpp"
#include "fish/scenes.hpp"
#include "fish/services.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

namespace fish
{
    SceneLoader::ASIOHandler::ASIOHandler(Services& services)
        : services(services), assets(services.getService<Assets>())
    {}

    bool SceneLoader::ASIOHandler::Exists(const char* file) const
    {
        return assets.exists(file);
    }

    char SceneLoader::ASIOHandler::getOsSeparator() const
    {
        return '/';
    }

    Assimp::IOStream* SceneLoader::ASIOHandler::Open(const char* file, const char* _)
    {
        return new SceneLoader::ASIOHandler::ASIOStream(assets.findAsset(file));
    }

    void SceneLoader::ASIOHandler::Close(Assimp::IOStream* stream)
    {
        delete stream;
    }

    // IOStream
    SceneLoader::ASIOHandler::ASIOStream::ASIOStream(std::shared_ptr<Asset> asset)
        : asset(asset)
    {
    }

    size_t SceneLoader::ASIOHandler::ASIOStream::Read(void* buffer, size_t size, size_t count)
    {
        auto& data = this->asset->bytes();
        size_t bytes = std::min(size * count, data.size() - cursor);

        std::memcpy(buffer, data.data() + cursor, bytes);

        return bytes / size;
    }

    size_t SceneLoader::ASIOHandler::ASIOStream::Write(const void* pvBuffer, size_t pSize, size_t pCount)
    {
// does NOTHING. you cannot WRITE an ASSET. IDIOT.
        return 0;
    }

    aiReturn SceneLoader::ASIOHandler::ASIOStream::Seek(size_t pOffset, aiOrigin pOrigin)
    {
        auto& data = this->asset->bytes();

        switch (pOrigin) {
        case aiOrigin_SET:
            this->cursor = pOffset;
            break;
        case aiOrigin_CUR:
            this->cursor += pOffset;
            break;
        case aiOrigin_END:
            this->cursor = data.size() - this->cursor;
            break;
        default:
            break;
        }
        return aiReturn_SUCCESS;
    }

    size_t SceneLoader::ASIOHandler::ASIOStream::Tell() const
    {
        return this->cursor;
    }

    size_t SceneLoader::ASIOHandler::ASIOStream::FileSize() const
    {
        return asset->size() * sizeof(unsigned char);
    }

    void SceneLoader::ASIOHandler::ASIOStream::Flush()
    {
// does NOTHING. FUCK YOU.
    }
}