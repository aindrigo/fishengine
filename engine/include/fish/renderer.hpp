#pragma once

#include "GLFW/glfw3.h"
#include "entt/entity/fwd.hpp"
#include "fish/camera.hpp"
#include "fish/lights.hpp"
#include "fish/material.hpp"
#include "fish/model.hpp"
#include "fish/services.hpp"
#include "fish/system.hpp"
#include "fish/texture.hpp"
#include "fish/transform.hpp"
#include "fish/world.hpp"
#include "glad/gl.h"
#include "fish/shaders.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "services.hpp"
#include <optional>
#include <vector>

#define MAX_POINTLIGHTS 256
namespace fish
{
    class Screen
    {
    public:
        Screen(GLFWwindow* window);
        void init();
        void clearScreen();

        GLuint getColorBuffer();
        GLuint getDepthBuffer();
        GLuint getFrameBuffer();
    private:
        GLuint colorBuffer;
        GLuint depthBuffer;
        GLuint frameBuffer;

        GLFWwindow* window;
    };

    class Renderer3D : public ISystem
    {
    public:
        Renderer3D(Services& services, GLFWwindow* window);
        void init() override;
        void render() override;
        void tick() override;
        void shutdown() override;
    private:
        static constexpr unsigned int gridSizeX = 16;
        static constexpr unsigned int gridSizeY = 16;
        static constexpr unsigned int numClusters = gridSizeX * gridSizeY;
        struct RenderPassData {
            struct RenderEntity {
                entt::entity entity;
                Mesh& mesh;
                Material& material;
                Transform3D& transform;
                Transform3D worldSpace;
                VertexGPUData& gpuData;
                glm::mat4 model;
                glm::mat4 mvp;
            };

            struct RenderCamera {
                entt::entity entity;
                Camera3D& camera;
                Transform3D worldSpace;
                glm::mat4 perspective;
                glm::mat4 view;
            };

            int width;
            int height;
            RenderCamera camera;

            std::vector<RenderEntity> entities;
        };

        void initBuffers();
        void onMeshCreate(entt::entity entity);
        void onMeshDestroy(entt::entity entity);
        void onDirLightCreate(entt::entity entity);
        void onDirLightDestroy(entt::entity entity);
        void doRender(int width, int height);
        void geometryPass(RenderPassData& data);
        void lightingPass(RenderPassData& data);
        void updateLights();
        GLuint createGBufferTexture(int width, int height, GLenum attachment, GLenum format);
        
        GLuint lightSSBO;

        GLuint gBuffer;
        GLuint gDepth;
        GLuint gPosition;
        GLuint gNormal;
        GLuint gAlbedoSpec;
        GLuint gModelPosition;
        GLuint rbo;
        
        
        VertexGPUData rect;
        std::optional<DirectionalLight> dirLight;
        Screen& screen;
        TextureManager& textureManager;
        ShaderManager& shaderCache;
        Services& services;
        World& world;
        GLFWwindow* window;
    };

    class Renderer2D : public ISystem
    {
    public:
        Renderer2D(Services& services, GLFWwindow* window);
        void init() override;
        void render() override;
        void shutdown() override;
        static VertexGPUData createRect(float scale = 0.5f);
    private:
        void doRender(int width, int height);

        VertexGPUData rect;
        TextureManager& textureManager;
        ShaderManager& shaderCache;
        Services& services;
        World& world;
        GLFWwindow* window;
    };

    
}