#include "fish/assets.hpp"
#include "fish/common.hpp"
#include "fish/renderer.hpp"
#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>
#include <vector>

namespace fish
{
    Renderer::ShaderCache::ShaderCache(Assets& assets)
        : assets(assets)
    {}

    unsigned int Renderer::ShaderCache::getShader(const std::string& name)
    {
        FISH_ASSERTF(this->assets.exists(std::filesystem::path("shaders") / std::filesystem::path(name)), "Shader {} does not exist", name);
        
        if (!this->shaders.contains(name))
            this->createShader(name);

        return this->shaders.at(name).program;
    }

    void Renderer::ShaderCache::createShader(const std::string& name)
    {
        std::filesystem::path shaderPath = std::filesystem::path("shaders") / std::filesystem::path(name);
        std::filesystem::path vertPath = shaderPath / std::filesystem::path("shader.vert");
        FISH_ASSERTF(this->assets.exists(vertPath), "shader.vert does not exist for shader {}", name);
        
        std::filesystem::path fragPath = shaderPath / std::filesystem::path("shader.frag");
        FISH_ASSERTF(this->assets.exists(fragPath), "shader.frag does not exist for shader {}", name);

        std::string vertShaderStr = this->assets.findAssetString(vertPath);
        std::string fragShaderStr = this->assets.findAssetString(fragPath);
        
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        
        ShaderCompilationResult vertResult = this->compileShader(vertexShader, vertShaderStr);
        FISH_ASSERTF(vertResult.success, "Shader {} did not compile successfully (vertex): {}", name, vertResult.message);

        ShaderCompilationResult fragResult = this->compileShader(fragmentShader, fragShaderStr);
        FISH_ASSERTF(fragResult.success, "Shader {} did not compile successfully (fragment): {}", name, fragResult.message);

        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        int success;
        if (success == 0) {
            int length;
            glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &length);

            std::vector<char> infoLogList(length);
            glGetProgramInfoLog(shaderProgram, length, &length, infoLogList.data());

            throw std::runtime_error(std::format("Shader {} linkage error: {}", name, std::string(infoLogList.begin(), infoLogList.end())));
        }

        this->shaders[name] = {
            .program = shaderProgram
        };
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    Renderer::ShaderCache::ShaderCompilationResult Renderer::ShaderCache::compileShader(unsigned int shader, const std::string& code)
    {
        const char* shaderCode = code.c_str();

        glShaderSource(shader, 1, &shaderCode, nullptr);
        glCompileShader(shader);

        ShaderCompilationResult result;
        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (success == 0) {
            int length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

            std::vector<char> infoLogList(length);
            glGetShaderInfoLog(shader, length, &length, infoLogList.data());

            result.success = false;
            result.message = std::string(infoLogList.begin(), infoLogList.end());
        } else {
            result.success = true;
        }

        return result;
    }

    void Renderer::ShaderCache::shutdown()
    {
        for (auto const& [name, shader] : this->shaders)
        {
            glDeleteShader(shader.program);
        }
    }
}