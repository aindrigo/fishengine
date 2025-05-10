#include "GLFW/glfw3.h"
#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"
#include "fish/camera.hpp"
#include "fish/common.hpp"
#include "fish/engineinfo.hpp"
#include "fish/material.hpp"
#include "fish/model.hpp"
#include "fish/node.hpp"
#include "fish/renderer.hpp"
#include "fish/services.hpp"
#include "fish/texture.hpp"
#include "fish/transform.hpp"
#include "fish/world.hpp"
#include "fish/helpers.hpp"
#include "fish/lights.hpp"
#include "glm/matrix.hpp"
#include <optional>
#include <vector>

namespace fish
{
    Renderer3D::Renderer3D(Services& services, GLFWwindow* window)
        : services(services), window(window), shaderCache(services.getService<ShaderManager>()),
          textureManager(services.getService<TextureManager>()), world(services.getService<World>()),
          screen(services.getService<Screen>())
    {}

    void Renderer3D::init()
    {
        this->initBuffers();
        auto& world = services.getService<World>();
        auto& engineInfo = services.getService<EngineInfo>();

        auto& registry = world.getRegistry();

        registry.on_construct<Mesh>().connect<&Renderer3D::onMeshCreate>(this);
        registry.on_destroy<Mesh>().connect<&Renderer3D::onMeshDestroy>(this);

        registry.on_construct<DirectionalLight>().connect<&Renderer3D::onDirLightCreate>(this);
        registry.on_destroy<DirectionalLight>().connect<&Renderer3D::onDirLightDestroy>(this);

        // depthbuffer
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
        this->initBuffers();
    }

    void Renderer3D::render()
    {
        int width;
        int height;
        glfwGetWindowSize(window, &width, &height);

        this->doRender(width, height);
    }

    void Renderer3D::tick()
    {
        textureManager.gc();
        auto& registry = world.getRegistry();

        // update lights
        // auto group = registry.group(entt::get<Node, PointLight, Transform3D>);
        // FISH_ASSERT(group.size() <= 256, "Cannot have more than 256 PointLights");

        // std::vector<PointLight::GLSL> pointLights (group.size());

        // for (auto& ent : group) {
        //     auto [_, light, transform] = group.get(ent);
        //     pointLights.push_back(light.toGL(world.worldSpace3DTransform(ent)));
        // }

        // glNamedBufferSubData(
        //     this->lightSSBO,
        //     0, sizeof(PointLight::GLSL) * pointLights.capacity(),
        //     pointLights.data()
        // );

        updateLights();
    }

    void Renderer3D::shutdown()
    {
        auto& registry = world.getRegistry();
        registry.on_construct<Mesh>().disconnect<&Renderer3D::onMeshCreate>(this);
        registry.on_destroy<Mesh>().disconnect<&Renderer3D::onMeshDestroy>(this);

        registry.on_construct<DirectionalLight>().disconnect<&Renderer3D::onDirLightCreate>(this);
        registry.on_destroy<DirectionalLight>().disconnect<&Renderer3D::onDirLightDestroy>(this);

        glDeleteVertexArrays(1, &this->rect.vao);
        glDeleteBuffers(1, &this->rect.vbo);
        glDeleteBuffers(1, &this->rect.ebo);
        glDeleteFramebuffers(1, &this->gBuffer);
    }

    void Renderer3D::initBuffers()
    {
        this->rect = Renderer2D::createRect(1.0f);
        glCreateBuffers(1, &this->lightSSBO);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, this->lightSSBO);
        glNamedBufferStorage(this->lightSSBO, sizeof(PointLight::GLSL) * MAX_POINTLIGHTS, nullptr, GL_DYNAMIC_STORAGE_BIT);
        
        // size
        int width;
        int height;
        glfwGetWindowSize(window, &width, &height);

        // gbuffer
        glCreateFramebuffers(1, &gBuffer);

        static const unsigned int attachments[] { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glNamedFramebufferDrawBuffers(gBuffer, sizeof(attachments) / sizeof(unsigned int), attachments);

        // textures
        glCreateTextures(GL_TEXTURE_2D, 1, &this->gDepth);
        glTextureStorage2D(this->gDepth, 1, GL_DEPTH_COMPONENT32F, width, height);
        glNamedFramebufferTexture(this->gBuffer, GL_DEPTH_ATTACHMENT, this->gDepth, 0);

        this->gPosition = createGBufferTexture(width, height, GL_COLOR_ATTACHMENT0, GL_RGB16F);
        this->gNormal = createGBufferTexture(width, height, GL_COLOR_ATTACHMENT1, GL_RGB16F);
        this->gAlbedoSpec = createGBufferTexture(width, height, GL_COLOR_ATTACHMENT2, GL_RGB16F);

        int status = glCheckNamedFramebufferStatus(this->gBuffer, GL_FRAMEBUFFER);
        FISH_ASSERTF(status == GL_FRAMEBUFFER_COMPLETE, "G-Buffer not FRAMEBUFFER_COMPLETE: {}", status);
    }

    void Renderer3D::onDirLightCreate(entt::entity entity)
    {
        auto& registry = world.getRegistry();

        this->dirLight = registry.get<DirectionalLight>(entity);
    }

    void Renderer3D::onDirLightDestroy(entt::entity entity)
    {
        this->dirLight = std::nullopt;
    }

    void Renderer3D::onMeshCreate(entt::entity entity)
    {
        auto& registry = this->world.getRegistry();
        if (registry.all_of<VertexGPUData>(entity))
            return;

        Mesh mesh = registry.get<Mesh>(entity);
        
        VertexGPUData data;
        glCreateBuffers(1, &data.vbo);
        glNamedBufferStorage(data.vbo, sizeof(Vertex) * mesh.vertices.size(), mesh.vertices.data(), GL_DYNAMIC_STORAGE_BIT);
        
        glCreateBuffers(1, &data.ebo);
        glNamedBufferStorage(data.ebo, sizeof(GLuint) * mesh.indices.size(), mesh.indices.data(), GL_DYNAMIC_STORAGE_BIT);
        
        glCreateVertexArrays(1, &data.vao);

        glVertexArrayVertexBuffer(data.vao, 0, data.vbo, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(data.vao, data.ebo);

        glEnableVertexArrayAttrib(data.vao, 0);
        glEnableVertexArrayAttrib(data.vao, 1);
        glEnableVertexArrayAttrib(data.vao, 2);
        glEnableVertexArrayAttrib(data.vao, 3);
        glEnableVertexArrayAttrib(data.vao, 4);

        glVertexArrayAttribFormat(data.vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribFormat(data.vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
        glVertexArrayAttribFormat(data.vao, 2, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, tangent));
        glVertexArrayAttribFormat(data.vao, 3, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, bitangent));
        glVertexArrayAttribFormat(data.vao, 4, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));

        glVertexArrayAttribBinding(data.vao, 0, 0);
        glVertexArrayAttribBinding(data.vao, 1, 0);
        glVertexArrayAttribBinding(data.vao, 2, 0);
        glVertexArrayAttribBinding(data.vao, 3, 0);
        glVertexArrayAttribBinding(data.vao, 4, 0);

        registry.emplace<VertexGPUData>(entity, data);
    }

    void Renderer3D::onMeshDestroy(entt::entity entity)
    {
        auto& registry = this->world.getRegistry();
        if (!registry.all_of<VertexGPUData>(entity))
            return;

        VertexGPUData data = registry.get<VertexGPUData>(entity);

        static const GLuint buffers[] = { data.ebo, data.vbo };

        glDeleteBuffers(2, buffers);
        glDeleteVertexArrays(1, &data.vao);
    }

    void Renderer3D::doRender(int width, int height)
    {
        glDepthFunc(GL_GREATER); // reversed-z stuff

        auto cameraEntity = world.getCamera();
        if (cameraEntity == entt::null)
            return;
        
        auto& registry = world.getRegistry();

        bool componentsFound = registry.all_of<Camera3D, Transform3D>(cameraEntity);
        FISH_ASSERT(componentsFound, "Camera3D & Transform3D not found on camera entity");

        auto& camera = registry.get<Camera3D>(cameraEntity);
        auto& world = services.getService<World>();

        auto cameraWorldTransform = world.worldSpace3DTransform(cameraEntity);
        glm::mat4 view = glm::inverse(cameraWorldTransform.build());

        glm::mat4 perspective = camera.build(static_cast<float>(width) / static_cast<float>(height));

        auto group = registry.group<Mesh>(entt::get<VertexGPUData, Material, Transform3D>);
        RenderPassData pass = {
            .width = width,
            .height = height,
            .camera = {
                .entity = cameraEntity,
                .camera = camera,
                .worldSpace = cameraWorldTransform,
                .perspective = perspective,
                .view = view
            }
        };

        
        for (auto const& ent : group) {
            RenderPassData::RenderEntity entity = {
                .entity = ent,
                .mesh = group.get<Mesh>(ent),
                .material = group.get<Material>(ent),
                .transform = group.get<Transform3D>(ent),
                .worldSpace = world.worldSpace3DTransform(ent),
                .gpuData = group.get<VertexGPUData>(ent),
            };

            entity.model = entity.worldSpace.build();
            entity.mvp = pass.camera.perspective * pass.camera.view * entity.model;
            pass.entities.push_back(entity);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        this->geometryPass(pass);

        glBindFramebuffer(GL_FRAMEBUFFER, screen.getFrameBuffer());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        this->lightingPass(pass);

        glBlitNamedFramebuffer(
            gBuffer, screen.getFrameBuffer(),
            width, height,
            0, 0,
            width, height,
            0, 0,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST
        );

        glDepthFunc(GL_LESS); // reversed-z
    }

    void Renderer3D::geometryPass(Renderer3D::RenderPassData& pass)
    {
        auto& engineInfo = this->services.getService<EngineInfo>();
        auto& world = this->services.getService<World>();
        auto& registry = world.getRegistry();

        unsigned int shader = this->shaderCache.getShader("3D_GeoPass");
            
        glUseProgram(shader);
        for (auto const& ent : pass.entities) {
            helpers::Uniform::uniformMatrix4x4(
                shader,
                "model",
                ent.model
            );

            helpers::Uniform::uniformMatrix4x4(
                shader,
                "view",
                pass.camera.view
            );
            
            helpers::Uniform::uniformMatrix4x4(
                shader,
                "perspective",
                pass.camera.perspective
            );

            helpers::Uniform::uniformMatrix4x4(
                shader,
                "mvp",
                ent.mvp
            );

            // DIFFUSE
            {
                TextureWrapMode wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
                if (ent.material.hasProperty<TextureWrapMode>("diffuseWrapMode")) {
                    wrapMode = ent.material.getProperty<TextureWrapMode>("diffuseWrapMode").value();
                }

                unsigned int id = textureManager.get(
                    ent.material.getProperty<std::string>("diffuseMap").value_or("textures/empty.png"), 
                    TextureFilterMode::LINEAR, 
                    wrapMode
                );
                textureManager.bind(id, 3);
                helpers::Uniform::uniformInt(shader, "diffuseMap", 3);
            }
            
            // NORMAL
            std::optional<std::string> normalMapOpt = ent.material.getProperty<std::string>("normalMap");

            if (normalMapOpt.has_value() && !normalMapOpt.value().empty()) {
                TextureWrapMode wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
                if (ent.material.hasProperty<TextureWrapMode>("normalWrapMode")) {
                    wrapMode = ent.material.getProperty<TextureWrapMode>("normalWrapMode").value();
                }

                unsigned int id = textureManager.get(
                    normalMapOpt.value(), 
                    TextureFilterMode::LINEAR, 
                    wrapMode
                );
                textureManager.bind(id, 4);
                helpers::Uniform::uniformInt(shader, "normalMap", 4);
                helpers::Uniform::uniformInt(shader, "hasNormalMap", 1);
            } else {
                helpers::Uniform::uniformInt(shader, "hasNormalMap", 0);
            }

            glBindVertexArray(ent.gpuData.vao);
            glDrawElements(GL_TRIANGLES, ent.mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);

            textureManager.bind(0, 3);
        }
    }

    void Renderer3D::lightingPass(Renderer3D::RenderPassData& pass)
    {
        // render
        unsigned int shader = this->shaderCache.getShader("3D_LightPass");

        glUseProgram(shader);

        textureManager.bind(gPosition, 0);
        textureManager.bind(gNormal, 1);
        textureManager.bind(gAlbedoSpec, 2);

        if (dirLight.has_value()) 
            helpers::Uniform::uniformDirectionalLight(shader, "dirLight", dirLight.value());
        else
            helpers::Uniform::uniformInt(shader, "dirLight.enabled", 0);
        
        helpers::Uniform::uniformInt(shader, "gPosition", 0);
        helpers::Uniform::uniformInt(shader, "gNormal", 1);
        helpers::Uniform::uniformInt(shader, "gAlbedoSpec", 2);
        helpers::Uniform::uniformInt(shader, "gModelPosition", 3);

        helpers::Uniform::uniformVec3(shader, "viewPos", pass.camera.worldSpace.position);

        glBindVertexArray(rect.vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void Renderer3D::updateLights()
    {
        auto& registry = this->world.getRegistry();
        auto group = registry.group(entt::get<Node, PointLight, Transform3D>);
        FISH_ASSERT(group.size() <= MAX_POINTLIGHTS, "Cannot have more than MAX_POINTLIGHTS pointlights in a scene");

        std::vector<PointLight::GLSL> lights(MAX_POINTLIGHTS);
        for (unsigned int i = 0; i < MAX_POINTLIGHTS; i++) {
            if (i >= group.size()) {
                lights[i] = {
                    .enabled = false
                };
                continue;
            }

            auto entity = group[i];

            auto& light = group.get<PointLight>(entity);
            auto glslLight = light.toGL(world.worldSpace3DTransform(entity));

            lights[i] = glslLight;
        }

        glNamedBufferSubData(this->lightSSBO, 0, sizeof(PointLight::GLSL) * MAX_POINTLIGHTS, lights.data());
    }

    GLuint Renderer3D::createGBufferTexture(int width, int height, GLenum attachment, GLenum format)
    {
        GLuint result;

        glCreateTextures(GL_TEXTURE_2D, 1, &result);
        glTextureParameteri(result, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(result, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(result, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(result, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTextureStorage2D(result, 1, format, width, height);
        glNamedFramebufferTexture(gBuffer, attachment, result, 0);
        
        return result;
    }
}