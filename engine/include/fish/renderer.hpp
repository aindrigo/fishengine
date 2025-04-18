#pragma once

#include "GLFW/glfw3.h"
#include "entt/entity/fwd.hpp"
#include "fish/services.hpp"
#include "fish/system.hpp"
#include "glad/gl.h"
#include "fish/model.hpp"
#include "services.hpp"
#include <string>
#include <unordered_map>

namespace fish
{
    class Renderer : public ISystem
    {
    public:
        Renderer(Services& services, GLFWwindow* window);
        void init() override; 
        void update() override;  
        void shutdown() override;
    private:
        class ShaderCache
        {
        public:
            ShaderCache(Services& services);
            unsigned int getShader(const std::string& name);
            void shutdown();
        private:
            
            void createShader(const std::string& name);

            struct ShaderCompilationResult {
                bool success;
                std::string message;
            };
            ShaderCompilationResult compileShader(unsigned int shader, const std::string& code);

            struct ShaderData {
                unsigned int program;
            };
            
            Services& services;
            std::unordered_map<std::string, ShaderData> shaders;
        };

        void render3D(int width, int height);
        void createRect();
        void render2D(int width, int height);
        void onMeshCreate(entt::entity entity);
        void onMeshDestroy(entt::entity entity);

        Services& services;
        VertexGPUData rect;
        GLFWwindow* window;
        entt::registry& registry;
        entt::entity cameraEntity;
        ShaderCache shaderCache;
    };
}