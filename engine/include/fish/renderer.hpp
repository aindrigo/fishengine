#pragma once

#include "GLFW/glfw3.h"
#include "entt/entity/fwd.hpp"
#include "fish/model.hpp"
#include "fish/services.hpp"
#include "fish/system.hpp"
#include "fish/texture.hpp"
#include "fish/world.hpp"
#include "glad/gl.h"
#include "fish/shaders.hpp"
#include "services.hpp"

namespace fish
{
    
    
    class Renderer3D : public ISystem
    {
    public:
        Renderer3D(Services& services, GLFWwindow* window);
        void init() override;
        void update() override;
        void shutdown() override;

        void preClearScreen();
    private:
        void initBuffers();
        void onMeshCreate(entt::entity entity);
        void onMeshDestroy(entt::entity entity);
        void render(int width, int height);


        
        TextureManager& textureManager;
        ShaderCache& shaderCache;
        Services& services;
        World& world;
        GLFWwindow* window;
    };

    class Renderer2D : public ISystem
    {
    public:
        Renderer2D(Services& services, GLFWwindow* window);
        void init() override;
        void update() override;
        void shutdown() override;
    private:
        void render(int width, int height);
        void createRect();

        VertexGPUData rect;
        TextureManager& textureManager;
        ShaderCache& shaderCache;
        Services& services;
        World& world;
        GLFWwindow* window;
    };

    class Screen
    {
    public:
        Screen(GLFWwindow* window);
        void init();
        void clearScreen();
    private:
        GLuint colorBuffer;
        GLuint depthBuffer;
        GLuint frameBuffer;

        GLFWwindow* window;
    };
}