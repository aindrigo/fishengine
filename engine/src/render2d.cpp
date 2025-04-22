#include "GLFW/glfw3.h"
#include "fish/model.hpp"
#include "fish/renderer.hpp"
#include "fish/helpers.hpp"

namespace fish
{
    Renderer2D::Renderer2D(Services& services, GLFWwindow* window)
        : services(services), window(window), shaderCache(services.getService<ShaderManager>()),
          textureManager(services.getService<TextureManager>()), world(services.getService<World>())
    {}

    void Renderer2D::init()
    {
        this->rect = createRect();
    }

    void Renderer2D::render()
    {
        int width;
        int height;
        glfwGetWindowSize(window, &width, &height);

        this->doRender(width, height);
    }

    void Renderer2D::shutdown()
    {
        glDeleteBuffers(1, &rect.vao);
        glDeleteBuffers(1, &rect.vbo);
        glDeleteBuffers(1, &rect.ebo);
    }

    void Renderer2D::doRender(int width, int height)
    {
        auto& registry = world.getRegistry();
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
                material.getProperty<Color>("diffuseColor").value_or(Color {})
            );

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
            glUseProgram(0);
        }
    }

    VertexGPUData Renderer2D::createRect(float scale)
    {
        VertexGPUData rect = {};
        Vertex2D vertices[] = {
            { { -scale, -scale }, { 0.0f, 0.0f } },
            { { -scale, scale }, { 0.0f, 1.0f } },
            { { scale, scale, }, { 1.0f, 1.0f } },
            { { scale, -scale }, { 1.0f, 0.0f } }
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

        glVertexArrayVertexBuffer(rect.vao, 0, rect.vbo, 0, sizeof(Vertex2D));
        
        glVertexArrayElementBuffer(rect.vao, rect.ebo);

        glEnableVertexArrayAttrib(rect.vao, 0);
        glEnableVertexArrayAttrib(rect.vao, 1);

        glVertexArrayAttribFormat(rect.vao, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex2D, position));
        glVertexArrayAttribFormat(rect.vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex2D, texCoord));

        glVertexArrayAttribBinding(rect.vao, 0, 0);
        glVertexArrayAttribBinding(rect.vao, 1, 0);

        return rect;
    }
}