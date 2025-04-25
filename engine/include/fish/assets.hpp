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
        bool isDirectory(const std::filesystem::path& dirPath);
        bool isFile(const std::filesystem::path& filePath);
        std::vector<std::filesystem::path> listDirectory(const std::filesystem::path& dirPath);
        std::string findAssetString(const std::filesystem::path& assetPath);
        std::vector<unsigned char> findAssetBytes(const std::filesystem::path& assetPath);
    private:
        std::filesystem::path assetDirectory;
    };
}