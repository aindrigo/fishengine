#pragma once

#include "assimp/matrix4x4.h"
#include "fish/common.hpp"
#include "glad/gl.h"

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
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

        static void uniformInt(unsigned int shader, std::string location, int data)
        {
            glUniform1i(
                glGetUniformLocation(shader, location.c_str()),
                data
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

    class Math
    {
    public:
        static bool inBounds(glm::vec2 original, std::pair<glm::vec2, glm::vec2> bounds)
        {
            return original.x >= bounds.first.x && original.x <= bounds.second.x &&
                   original.y >= bounds.first.y && original.y <= bounds.second.y;
                // the symmetry... its beautiful...
        }

        static glm::mat4 toGLM(aiMatrix4x4& matrix) {
            glm::mat4 result;
            result[0][0] = matrix.a1; result[1][0] = matrix.a2; result[2][0] = matrix.a3; result[3][0] = matrix.a4;
            result[0][1] = matrix.b1; result[1][1] = matrix.b2; result[2][1] = matrix.b3; result[3][1] = matrix.b4;
            result[0][2] = matrix.c1; result[1][2] = matrix.c2; result[2][2] = matrix.c3; result[3][2] = matrix.c4;
            result[0][3] = matrix.d1; result[1][3] = matrix.d2; result[2][3] = matrix.d3; result[3][3] = matrix.d4;
            return result;
        }

        // https://nlguillemot.wordpress.com/2016/12/07/reversed-z-in-opengl/
        static glm::mat4 perspectiveReverseZ(float fovy, float aspect, float zNear)
        {
            float f = 1.0f / tan(fovy / 2.0f);
            return glm::mat4(
                f / aspect, 0.0f,  0.0f,  0.0f,
                        0.0f,    f,  0.0f,  0.0f,
                        0.0f, 0.0f,  0.0f, -1.0f,
                        0.0f, 0.0f, zNear,  0.0f);
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