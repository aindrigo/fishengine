#pragma once

#include "GLFW/glfw3.h"
#include "entt/entity/fwd.hpp"
#include "fish/assets.hpp"
#include "fish/system.hpp"
#include "glad/gl.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace fish
{
    class Renderer : public ISystem
    {
    public:
        Renderer(Assets& assets, GLFWwindow* window);
        void init(std::weak_ptr<entt::registry> regPtr) override; 
        void update() override;  
        void shutdown() override;
    private:
        class ShaderCache
        {
        public:
            ShaderCache(Assets& assets);
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
            
            Assets& assets;
            std::unordered_map<std::string, ShaderData> shaders;
        };

        GLFWwindow* window;
        entt::entity cameraEntity;
        void onMeshCreate(entt::entity entity);
        void onMeshDestroy(entt::entity entity);
        ShaderCache shaderCache;
        std::weak_ptr<entt::registry> regPtr;
    };
}