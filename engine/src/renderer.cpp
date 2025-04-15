#include "fish/renderer.hpp"
#include "GLFW/glfw3.h"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "fish/assets.hpp"
#include "fish/transform.hpp"
#include "glad/gl.h"
#include "fish/common.hpp"
#include "fish/mesh.hpp"
#include "fish/material.hpp"
#include "fish/camera.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <memory>

namespace fish
{
    Renderer::Renderer(Assets& assets, GLFWwindow* window)
        : shaderCache(assets), window(window)
    {}

    void Renderer::init(std::weak_ptr<entt::registry> regPtr)
    {
        this->regPtr = regPtr;
        std::shared_ptr<entt::registry> registry = this->regPtr.lock();
        FISH_ASSERT(registry != nullptr, "Could not acquire lock on registry for renderer, not updating.");

        this->cameraEntity = registry->create();
        registry->emplace<Camera3D>(this->cameraEntity);

        registry->on_construct<Mesh>().connect<&Renderer::onMeshCreate>(this);
        registry->on_destroy<Mesh>().connect<&Renderer::onMeshDestroy>(this);
    }

    void Renderer::update()
    {
        FISH_ASSERT(!this->regPtr.expired(), "Registry pointer expired, cannot update");
        auto registry = this->regPtr.lock();
        FISH_ASSERT(registry != nullptr, "Could not acquire lock on registry for renderer, not updating.");
        
        // camera
        FISH_ASSERT(registry->all_of<Camera3D>(this->cameraEntity), "Camera3D not found on camera entity");
        Camera3D& camera = registry->get<Camera3D>(cameraEntity);
        
        // window
        int width;
        int height;
        glfwGetWindowSize(this->window, &width, &height);
        
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

        auto view = registry->view<Mesh, MeshGPUData, Material, Transform>();
        for (auto const& ent : view) {
            Mesh mesh = view.get<Mesh>(ent);
            MeshGPUData data = view.get<MeshGPUData>(ent);
            Material material = view.get<Material>(ent);
            Transform transform = view.get<Transform>(ent);

            unsigned int shader = this->shaderCache.getShader(material.shader);
            
            glUseProgram(shader);
            glUniform3f(
                glGetUniformLocation(shader, "cameraPosition"),
                camera.transform.position.x,
                camera.transform.position.y,
                camera.transform.position.z
            );

            glUniformMatrix4fv(
                glGetUniformLocation(shader, "model"),
                1,
                GL_FALSE,
                glm::value_ptr(transform.build())
            );

            glUniformMatrix4fv(
                glGetUniformLocation(shader, "view"),
                1,
                GL_FALSE,
                glm::value_ptr(camera.transform.build())
            );
            
            glUniformMatrix4fv(
                glGetUniformLocation(shader, "projection"),
                1,
                GL_FALSE,
                glm::value_ptr(camera.build(aspectRatio))
            );
            
            glBindVertexArray(data.vao);
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
            glUseProgram(0);
        }
    }

    void Renderer::shutdown()
    {
        auto registry = this->regPtr.lock();
        FISH_ASSERT(registry != nullptr, "Could not acquire lock on registry for renderer, not updating.");

        registry->on_construct<Mesh>().disconnect<&Renderer::onMeshCreate>(this);
        registry->on_destroy<Mesh>().disconnect<&Renderer::onMeshDestroy>(this);

        this->shaderCache.shutdown();
    }

    void Renderer::onMeshCreate(entt::entity entity)
    {
        auto registry = this->regPtr.lock();
        FISH_ASSERT(registry != nullptr, "Could not acquire lock on registry for renderer, not updating.");

        if (registry->all_of<MeshGPUData>(entity))
            return;

        Mesh mesh = registry->get<Mesh>(entity);
        
        MeshGPUData data;
        glGenVertexArrays(1, &data.vao);
        glGenBuffers(1, &data.vbo);
        glGenBuffers(1, &data.ebo);

        glBindVertexArray(data.vao);

        glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(GLuint), mesh.indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        registry->emplace<MeshGPUData>(entity, data);
    }

    void Renderer::onMeshDestroy(entt::entity entity)
    {
        auto registry = this->regPtr.lock();
        FISH_ASSERT(registry != nullptr, "Could not acquire lock on registry for renderer, not updating.");


        if (!registry->all_of<MeshGPUData>(entity))
            return;

        MeshGPUData data = registry->get<MeshGPUData>(entity);

        glDeleteBuffers(2, new GLuint[2] { data.ebo, data.vbo });
        glDeleteVertexArrays(1, &data.vao);
    }
}
