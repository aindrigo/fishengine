#include "GLFW/glfw3.h"
#include "fish/camera.hpp"
#include "fish/model.hpp"
#include "fish/renderer.hpp"
#include "fish/services.hpp"
#include "fish/texture.hpp"
#include "fish/world.hpp"
#include "fish/helpers.hpp"

namespace fish
{
    Renderer3D::Renderer3D(Services& services, GLFWwindow* window)
        : services(services), window(window), shaderCache(services.getService<ShaderCache>()),
          textureManager(services.getService<TextureManager>()), world(services.getService<World>())
    {}

    void Renderer3D::init()
    {
        this->initBuffers();
        auto& world = services.getService<World>();
        auto& engineInfo = services.getService<EngineInfo>();

        auto& registry = world.getRegistry();

        registry.on_construct<Mesh>().connect<&Renderer3D::onMeshCreate>(this);
        registry.on_destroy<Mesh>().connect<&Renderer3D::onMeshDestroy>(this);

        // depthbuffer
        glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
        this->initBuffers();
    }

    void Renderer3D::update()
    {
        int width;
        int height;
        glfwGetWindowSize(window, &width, &height);

        this->render(width, height);
    }

    void Renderer3D::shutdown()
    {
        auto& registry = world.getRegistry();
        registry.on_construct<Mesh>().disconnect<&Renderer3D::onMeshCreate>(this);
        registry.on_destroy<Mesh>().disconnect<&Renderer3D::onMeshDestroy>(this);
    }

    void Renderer3D::initBuffers()
    {
        // TODO: tiled deferred rendering stuff
    }
    
    void Renderer3D::render(int width, int height)
    {
        glDepthFunc(GL_GREATER); // reversed-z stuff

        auto& engineInfo = this->services.getService<EngineInfo>();
        if (!engineInfo.camera.has_value())
            return;

        auto& registry = world.getRegistry();
        auto& cameraEntity = engineInfo.camera.value();

        bool componentsFound = registry.all_of<Camera3D, Transform3D>(cameraEntity);
        FISH_ASSERT(componentsFound, "Camera3D & Transform3D not found on camera entity");

        auto [camera, cameraTransform]= registry.get<Camera3D, Transform3D>(cameraEntity);
        auto& world = services.getService<World>();

        auto group = registry.group<Mesh>(entt::get<VertexGPUData, Material, Transform3D>);
        glm::mat4 view = cameraTransform.build();
        glm::mat4 projection = camera.build(static_cast<float>(width) / static_cast<float>(height));

        for (auto const& ent : group) {
            auto [mesh, data, material, transform] = group.get(ent);
            
            unsigned int shader = this->shaderCache.getShader(material.shader);
            
            Transform3D worldSpace = world.worldSpace3DTransform(ent);
            glUseProgram(shader);

            auto model = worldSpace.build();
            auto view = cameraTransform.build();

            helpers::Uniform::uniformVec3(
                shader, 
                "cameraPosition", 
                cameraTransform.position
            );

            helpers::Uniform::uniformMatrix4x4(
                shader,
                "model",
                model
            );

            helpers::Uniform::uniformMatrix4x4(
                shader,
                "view",
                view
            );
            
            helpers::Uniform::uniformMatrix4x4(
                shader,
                "projection",
                camera.build(static_cast<float>(width) / static_cast<float>(height))
            );

            helpers::Uniform::uniformMatrix4x4(
                shader,
                "mvp",
                projection * view * model
            );

            // DIFFUSE
            {
                TextureWrapMode wrapMode = TextureWrapMode::CLAMP_TO_EDGE;
                if (material.hasProperty<TextureWrapMode>("diffuseWrapMode")) {
                    wrapMode = material.getProperty<TextureWrapMode>("diffuseWrapMode").value();
                }

                unsigned int id = textureManager.get(
                    material.getProperty<std::string>("diffuseMap").value_or("textures/empty.png"), 
                    TextureFilterMode::LINEAR, 
                    wrapMode
                );
                textureManager.bind(id, 1);
                helpers::Uniform::uniformInt(shader, "diffuseMap", 1);
            }

            glBindVertexArray(data.vao);
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
            glUseProgram(0);
        }

        glDepthFunc(GL_LESS); // reversed-z
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

        glDeleteBuffers(2, new GLuint[2] { data.ebo, data.vbo });
        glDeleteVertexArrays(1, &data.vao);
    }
}