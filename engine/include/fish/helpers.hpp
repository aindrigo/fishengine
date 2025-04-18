#pragma once

#include "fish/common.hpp"
#include "glad/gl.h"

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "fish/material.hpp"
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>

#ifdef FISH_WINDOWS
#include <windows.h>
#endif

namespace fish::helpers
{
    class Uniform
    {
    public:
        static void uniformVec3(unsigned int shader, std::string location, glm::vec3 data)
        {
            glUniform3f(
                glGetUniformLocation(shader, location.c_str()),
                data.x,
                data.y,
                data.z
            );
        }
        static void uniformMatrix4x4(unsigned int shader, std::string location, glm::mat4x4 data)
        {
            glUniformMatrix4fv(
                glGetUniformLocation(shader, location.c_str()),
                1,
                GL_FALSE,
                glm::value_ptr(data)
            );
        }

        static void uniformColor(unsigned int shader, std::string location, Color data)
        {
            glUniform4f(
                glGetUniformLocation(shader, location.c_str()),
                data.r / 255.0f,
                data.g / 255.0f,
                data.b / 255.0f,
                data.a / 255.0f
            );
        }
    };

    class File
    {
    public:
        static std::string readFile(const std::filesystem::path& path)
        {
            FISH_ASSERTF(std::filesystem::is_regular_file(path), "File {} does not exist or is not a file", path.string());
            std::ifstream stream(path, std::ios_base::binary);
            stream.exceptions(std::ifstream::badbit | std::ifstream::failbit);

            std::stringstream strstream;
            strstream << stream.rdbuf();

            return strstream.str();
        }
    };

    static void fatalError(const std::string& message, const std::string& caption = "Fatal Error")
    {
#ifdef FISH_WINDOWS
        MessageBoxA(
            NULL,
            message.c_str(),
            caption.c_str(),
            MB_ICONERROR | MB_OK | MB_DEFBUTTON2
        );
#endif
        std::cout << std::format("Fatal error caught ({}): {}", caption, message) << std::endl;
        exit(EXIT_FAILURE);
    }
}