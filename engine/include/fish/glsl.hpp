#pragma once

#include "fish/assets.hpp"
#include <optional>
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
        std::optional<std::string> findFile(const std::string& fileName, const std::vector<std::string>& paths);
        static constexpr std::string_view globalSearchDirs[] = {
            "shaders/Libraries"
        };
        
        Assets& assets;
    };
}