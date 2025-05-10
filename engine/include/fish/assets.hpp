#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
namespace fish
{
    class Asset
    {
    public:
        Asset(const std::vector<unsigned char>& data, const std::filesystem::path& path);

        size_t size();
        std::vector<unsigned char>& bytes();
        std::string str();
        std::filesystem::path& path();
    private:
        std::filesystem::path assetPath;
        std::vector<unsigned char> data;
        friend class Assets;
    };

    class Assets
    {
    public:
        Assets(const std::filesystem::path& assetDirectory);
        bool exists(const std::filesystem::path& assetPath);
        bool isDirectory(const std::filesystem::path& dirPath);
        bool isFile(const std::filesystem::path& filePath);
        std::vector<std::filesystem::path> listDirectory(const std::filesystem::path& dirPath);

        std::shared_ptr<Asset> findAsset(const std::filesystem::path& assetPath);
    private:
        std::filesystem::path assetDirectory;
    };
}