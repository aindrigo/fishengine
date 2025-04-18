#include "fish/renderer.hpp"
#include "GLFW/glfw3.h"
#include "entt/entt.hpp"
#include "fish/engineinfo.hpp"
#include "fish/services.hpp"
#include "fish/transform.hpp"
#include "fish/world.hpp"
#include "glad/gl.h"
#include "fish/common.hpp"
#include "fish/model.hpp"
#include "fish/material.hpp"
#include "fish/helpers.hpp"
#include "fish/camera.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"

namespace fish
{
    Renderer::Renderer(Services& services, GLFWwindow* window)
        : services(services), window(window), registry(services.getService<World>().getRegistry()), shaderCache(services)
    {}

    void Renderer::init()
    {
        auto& world = services.getService<World>();
        auto& engineInfo = services.getService<EngineInfo>();

        engineInfo.camera = world.create();
        registry.emplace<Camera3D>(engineInfo.camera.value());
        registry.emplace<Transform3D>(engineInfo.camera.value());
        registry.on_construct<Mesh>().connect<&Renderer::onMeshCreate>(this);
        registry.on_destroy<Mesh>().connect<&Renderer::onMeshDestroy>(this);
        this->createRect();
    }

    void Renderer::update()
    {
        int width;
        int height;
        glfwGetWindowSize(this->window, &width, &height);

        // camera
        this->render2D(width, height);
        this->render3D(width, height);
    }

    void Renderer::shutdown()
    {
        registry.on_construct<Mesh>().disconnect<&Renderer::onMeshCreate>(this);
        registry.on_destroy<Mesh>().disconnect<&Renderer::onMeshDestroy>(this);

        this->shaderCache.shutdown();
    }

    void Renderer::render3D(int width, int height)
    {
        auto& engineInfo = this->services.getService<EngineInfo>();
        if (!engineInfo.camera.has_value())
            return;

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
            helpers::Uniform::uniformVec3(
                shader, 
                "cameraPosition", 
                cameraTransform.position
            );

            helpers::Uniform::uniformMatrix4x4(
                shader,
                "model",
                worldSpace.build()
            );

            helpers::Uniform::uniformMatrix4x4(
                shader,
                "view",
                cameraTransform.build()
            );
            
            helpers::Uniform::uniformMatrix4x4(
                shader,
                "projection",
                camera.build(static_cast<float>(width) / static_cast<float>(height))
            );
            
            glBindVertexArray(data.vao);
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
            glUseProgram(0);
        }
    }

    void Renderer::createRect()
    {
        float scale = 0.5f;
        float vertices[] = {
            scale, scale, 0.0f,
            scale, -scale, 0.0f,
            -scale,  -scale, 0.0f,
            -scale, scale, 0.0f
        };

        int indices[] = {
            0, 1, 3,
            1, 2, 3
        };

        glCreateBuffers(1, &rect.vbo);
        glNamedBufferStorage(rect.vbo, sizeof(vertices), vertices, GL_DYNAMIC_STORAGE_BIT);
        
        glCreateBuffers(1, &rect.ebo);
        glNamedBufferStorage(rect.ebo, sizeof(indices), indices, GL_DYNAMIC_STORAGE_BIT);
        
        glCreateVertexArrays(1, &rect.vao);

        glVertexArrayVertexBuffer(rect.vao, 0, rect.vbo, 0, 3 * sizeof(float));
        glVertexArrayElementBuffer(rect.vao, rect.ebo);

        glEnableVertexArrayAttrib(rect.vao, 0);

        glVertexArrayAttribFormat(rect.vao, 0, 3, GL_FLOAT, GL_FALSE, 0);

        glVertexArrayAttribBinding(rect.vao, 0, 0);
    }

    void Renderer::render2D(int width, int height)
    {
        auto& world = services.getService<World>();

        auto group = registry.group<Panel>(entt::get<Transform2D, Material>);
        group.sort<Transform2D>([](const Transform2D& lhs, const Transform2D& rhs) {
            return lhs.z > rhs.z;
        });
        glm::mat4 ortho = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);

        for (auto const& ent : group) {
            auto [panel, transform, material] = group.get(ent);

            Transform2D worldSpace = world.worldSpace2DTransform(ent);

            GLuint shader = this->shaderCache.getShader(material.shader);

            glUseProgram(shader);
            glBindVertexArray(rect.vao);
            
            helpers::Uniform::uniformMatrix4x4(
                shader,
                "perspective",
                ortho
            );
            helpers::Uniform::uniformMatrix4x4(
                shader,
                "model",
                worldSpace.build()
            );

            helpers::Uniform::uniformColor(
                shader,
                "color",
                material.getProperty<Color>("color").value_or(Color {})
            );

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
            glUseProgram(0);
        }
    }

    void Renderer::onMeshCreate(entt::entity entity)
    {
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

    void Renderer::onMeshDestroy(entt::entity entity)
    {
        if (!registry.all_of<VertexGPUData>(entity))
            return;

        VertexGPUData data = registry.get<VertexGPUData>(entity);

        glDeleteBuffers(2, new GLuint[2] { data.ebo, data.vbo });
        glDeleteVertexArrays(1, &data.vao);
    }
}
