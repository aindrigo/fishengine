#include "fish/assets.hpp"
#include "fish/common.hpp"
#include <filesystem>
#include <fstream>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fish
{
    Assets::Assets(const std::filesystem::path& assetDirectory)
        : assetDirectory(assetDirectory)
    {
        FISH_ASSERT(std::filesystem::is_directory(assetDirectory), "Asset directory not a directory or not found");
    }

    bool Assets::exists(const std::filesystem::path& assetPath)
    {
        std::filesystem::path fsPath = this->assetDirectory / assetPath;
        return std::filesystem::exists(fsPath);
    }

    bool Assets::isDirectory(const std::filesystem::path& dirPath)
    {
        std::filesystem::path fsPath = this->assetDirectory / dirPath;
        return std::filesystem::is_directory(fsPath);
    }

    bool Assets::isFile(const std::filesystem::path& filePath)
    {
        std::filesystem::path fsPath = this->assetDirectory / filePath;
        return std::filesystem::is_regular_file(fsPath) || std::filesystem::is_symlink(fsPath);
    }

    std::vector<std::filesystem::path> Assets::listDirectory(const std::filesystem::path& dirPath)
    {
        FISH_ASSERT(isDirectory(dirPath), "listDirectory(dirPath): dirPath must be a folder");

        std::filesystem::path fsPath = this->assetDirectory / dirPath;
        auto iterator = std::filesystem::directory_iterator(fsPath);
        std::vector<std::filesystem::path> files;

        for (auto const& entry : iterator)
            files.push_back(std::filesystem::relative(entry, this->assetDirectory));

        return files;
    }

    std::string Assets::findAssetString(const std::filesystem::path& assetPath)
    {
        FISH_ASSERTF(isFile(assetPath), "Asset {} does not exist or is not a file", assetPath.string());
        std::filesystem::path fsPath = this->assetDirectory / assetPath;

        std::ifstream stream(fsPath, std::ios_base::in);   
        std::stringstream strstream;
        strstream << stream.rdbuf();

        stream.close();
        return strstream.str();
    }

    std::vector<unsigned char> Assets::findAssetBytes(const std::filesystem::path& assetPath)
    {
        FISH_ASSERTF(isFile(assetPath), "Asset {} does not exist or is not a file", assetPath.string());
        std::filesystem::path fsPath = this->assetDirectory / assetPath;

        std::ifstream stream(fsPath, std::ios_base::binary | std::ios_base::in | std::ios_base::ate);
        stream.unsetf(std::ios_base::skipws);
        
        std::streampos size = stream.tellg();
        stream.seekg(0);
        
        std::vector<unsigned char> data(size);

        stream.read(reinterpret_cast<char*>(data.data()), size);
        stream.close();

        return data;
    }
}