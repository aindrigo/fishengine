#include "fish/glsl.hpp"
#include "fish/assets.hpp"
#include "fish/common.hpp"
#include "fish/helpers.hpp"
#include <filesystem>
#include <format>
#include <set>
#include <string>
#include <vector>

namespace fish
{
    GLSLPreProcessor::GLSLPreProcessor(Assets& assets)
        : assets(assets)
    {}

    GLSLPreProcessor::ProcessResult GLSLPreProcessor::process(const std::filesystem::path& file)
    {
        std::set<std::string> includedFiles;
        return internalProcess(file, includedFiles);
    }

    GLSLPreProcessor::ProcessResult GLSLPreProcessor::internalProcess(const std::filesystem::path& file, std::set<std::string>& includedFiles)
    {
        std::string code = this->assets.findAssetString(file);
        std::filesystem::path dir = file.parent_path();

        ProcessResult result;
        result.success = true;

        std::vector<std::string> lines = helpers::String::splitString(code, "\n");
        std::vector<std::string> linesResult = std::vector<std::string>(lines);

        unsigned int lineIndex = 0;
        for (auto const& line : lines) {
            if (!line.starts_with("#")) {
                lineIndex++;
                continue;
            }

            std::vector<std::string> words = helpers::String::splitString(line, " ");
            std::string& directive = words.at(0);

            if (directive == "#include") {
                if (words.size() != 2) {
                    result.success = false;
                    result.data = std::format("{}:{}: Invalid usage of #include (#include filepath)", file.string(), lineIndex);
                    break;
                }

                std::string fileStr = helpers::String::joinString(words, " ");
                auto delResult = helpers::String::getStringBetweenDelimiters(
                    fileStr, { '<', '"', '\'' }, { '>', '"', '\'' });

                if (!delResult.success) {
                    result.success = false;
                    result.data = std::format("{}:{}: Invalid quoted string in #include directive: {}", file.string(), lineIndex, fileStr);
                    break;
                }

                auto filePath = this->findFile(delResult.data, { dir.string() });

                if (!filePath) {
                    result.success = false;
                    result.data = std::format("{}:{}: Could not find file {}", file.string(), lineIndex, delResult.data);
                    break;
                }

                if (!includedFiles.contains(filePath.value())) {
                    ProcessResult includeResult = internalProcess(filePath.value(), includedFiles);

                    FISH_ASSERTF(includeResult.success, "{}", includeResult.data);

                    linesResult[lineIndex] = includeResult.data;
                    includedFiles.insert(filePath.value());
                } else {
                    linesResult[lineIndex] = "";
                }
            }

            lineIndex++;
        }

        if (result.success)
            result.data = helpers::String::joinString(linesResult, "\n");

        return result;
    }

    std::optional<std::string> GLSLPreProcessor::findFile(const std::string& file, const std::vector<std::string>& paths)
    {
        for (auto const& folder : this->globalSearchDirs) {
            std::string filePath = std::format("{}/{}", folder, file);
            if (!assets.exists(filePath))
                continue;

            return filePath;
        }

        for (auto const& folder : paths) {
            std::string filePath = std::format("{}/{}", folder, file);
            if (!assets.exists(filePath))
                continue;

            return filePath;
        }

        return std::nullopt;
    }
}