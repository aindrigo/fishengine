#include "fish/assets.hpp"
#include "fish/common.hpp"
#include <filesystem>
#include <fstream>
#include <iterator>
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

    std::string Assets::findAssetString(const std::filesystem::path& assetPath)
    {
        std::filesystem::path fsPath = this->assetDirectory / assetPath;

        FISH_ASSERTF(std::filesystem::is_regular_file(fsPath), "Asset {} does not exist or is not a file", assetPath.string());
        std::ifstream stream(fsPath);

        std::stringstream strstream;
        strstream << stream.rdbuf();

        return strstream.str();

    }

    std::vector<unsigned char> Assets::findAssetBytes(const std::filesystem::path& assetPath)
    {
        std::filesystem::path fsPath = this->assetDirectory / assetPath;

        FISH_ASSERTF(std::filesystem::is_regular_file(fsPath), "Asset {} does not exist or is not a file", assetPath.string());
        std::ifstream stream(fsPath);

        return std::vector<unsigned char>(
            (std::istreambuf_iterator<char>(stream)), 
            std::istreambuf_iterator<char>());
    }
}