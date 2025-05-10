#include "fish/assets.hpp"
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace fish
{
    Asset::Asset(const std::vector<unsigned char>& data, const std::filesystem::path& path)
        : data(data), assetPath(path)
    {}

    size_t Asset::size()
    {
        return this->data.size();
    }
    
    std::vector<unsigned char>& Asset::bytes()
    {
        return this->data;
    }

    std::string Asset::str()
    {
        return std::string(this->data.begin(), this->data.end());
    }

    std::filesystem::path& Asset::path()
    {
        return this->assetPath;
    }
}