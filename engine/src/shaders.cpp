#include "fish/assets.hpp"
#include "fish/common.hpp"
#include "fish/shaders.hpp"
#include "fish/glsl.hpp"
#include "fish/services.hpp"
#include "glad/gl.h"
#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>
#include <vector>

namespace fish
{
    ShaderManager::ShaderManager(Services& services)
        : services(services), processor(services.getService<Assets>())
    {}

    unsigned int ShaderManager::getShader(const std::string& name)
    {
        auto& assets = this->services.getService<Assets>();
        FISH_ASSERTF(assets.exists(std::filesystem::path("shaders") / std::filesystem::path(name)), "Shader {} does not exist", name);
        
        if (!this->shaders.contains(name))
            this->createShader(name);

        return this->shaders.at(name).program;
    }

    void ShaderManager::createShader(const std::string& name)
    {
        auto& assets = this->services.getService<Assets>();
        std::filesystem::path shaderPath = std::filesystem::path("shaders") / std::filesystem::path(name);
        std::filesystem::path vertPath = shaderPath / std::filesystem::path("shader.vert");        
        std::filesystem::path fragPath = shaderPath / std::filesystem::path("shader.frag");
        std::filesystem::path computePath = shaderPath / std::filesystem::path("shader.comp");        

        GLuint shaderProgram = glCreateProgram();

        GLuint vertexShader = 0;
        GLuint fragmentShader = 0;
        GLuint computeShader = 0;
        if (assets.exists(vertPath)) {
            vertexShader = glCreateShader(GL_VERTEX_SHADER);
            auto result = processor.process(vertPath);
            FISH_ASSERTF(result.success, "Shader {} did not preprocess successfully (vertex): {}", name, result.data);

            ShaderCompilationResult vertResult = this->compileShader(vertexShader, result.data);
            FISH_ASSERTF(vertResult.success, "Shader {} did not compile successfully (vertex): {}", name, vertResult.message);

            glAttachShader(shaderProgram, vertexShader);
        }
        
        if (assets.exists(fragPath)) {
            fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            auto result = processor.process(fragPath);
            FISH_ASSERTF(result.success, "Shader {} did not preprocess successfully (fragment): {}", name, result.data);

            ShaderCompilationResult fragResult = this->compileShader(fragmentShader, result.data);
            FISH_ASSERTF(fragResult.success, "Shader {} did not compile successfully (fragment): {}", name, fragResult.message);

            glAttachShader(shaderProgram, fragmentShader);
        }

        if (assets.exists(computePath)) {
            computeShader = glCreateShader(GL_COMPUTE_SHADER);
            std::string computeShaderStr = assets.findAssetString(computePath);
            
            ShaderCompilationResult computeResult = this->compileShader(computeShader, computeShaderStr);
            FISH_ASSERTF(computeResult.success, "Shader {} did not compile successfully (compute): {}", name, computeResult.message);

            glAttachShader(shaderProgram, computeShader);
        }

        glLinkProgram(shaderProgram);

        int success;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
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
        
        if (vertexShader != 0)
            glDeleteShader(vertexShader);

        if (fragmentShader != 0)
            glDeleteShader(fragmentShader);
        
        if (computeShader != 0)
            glDeleteShader(computeShader);
    }

    ShaderManager::ShaderCompilationResult ShaderManager::compileShader(unsigned int shader, const std::string& code)
    {
        const char* shaderCode = code.c_str();

        glShaderSource(shader, 1, &shaderCode, nullptr);
        glCompileShader(shader);

        ShaderCompilationResult result {};
        int success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (success == 0) {
            int length = 0;
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

    void ShaderManager::shutdown()
    {
        for (auto const& [name, shader] : this->shaders)
        {
            glDeleteProgram(shader.program);
        }
    }
}