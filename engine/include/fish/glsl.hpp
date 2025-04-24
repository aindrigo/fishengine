#pragma once

#include "fish/assets.hpp"
#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace fish
{
    class GLSLPreProcessor
    {
    public:
        struct ProcessResult {
            bool success;
            std::string data; // if success == true then code, otherwise error message
        };

        GLSLPreProcessor(Assets& assets);

        ProcessResult process(const std::filesystem::path& file);
    private:
        ProcessResult internalProcess(const std::filesystem::path& file, std::set<std::string>& includedFiles);
        std::optional<std::string> findFile(const std::string& fileName, const std::vector<std::string>& paths);
        static constexpr std::string_view globalSearchDirs[] = {
            "shaders/Libraries"
        };
        
        Assets& assets;
    };
}