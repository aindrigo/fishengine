#pragma once

#include <filesystem>
#include <string>
#include <vector>
namespace fish
{
    class Assets
    {
    public:
        Assets(const std::filesystem::path& assetDirectory);
        bool exists(const std::filesystem::path& assetPath);
        std::string findAssetString(const std::filesystem::path& assetPath);
        std::vector<unsigned char> findAssetBytes(const std::filesystem::path& assetPath);
    private:
        std::filesystem::path assetDirectory;
    };
}