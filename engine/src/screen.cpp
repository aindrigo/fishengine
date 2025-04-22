
#include "fish/helpers.hpp"
#include "fish/renderer.hpp"
#include "GLFW/glfw3.h"
namespace fish
{

    Screen::Screen(GLFWwindow* window)
        : window(window)
    {}

    void Screen::init()
    {
        int width;
        int height;
        glfwGetWindowSize(window, &width, &height);

        glCreateTextures(GL_TEXTURE_2D, 1, &this->colorBuffer);
        glTextureStorage2D(this->colorBuffer, 1, GL_SRGB8_ALPHA8, width, height);

        glCreateTextures(GL_TEXTURE_2D, 1, &this->depthBuffer);
        glTextureStorage2D(this->depthBuffer, 1, GL_DEPTH_COMPONENT32F, width, height);

        glCreateFramebuffers(1, &this->frameBuffer);
        glNamedFramebufferTexture(this->frameBuffer, GL_COLOR_ATTACHMENT0, this->colorBuffer, 0);
        glNamedFramebufferTexture(this->frameBuffer, GL_DEPTH_ATTACHMENT, this->depthBuffer, 0);
        
        GLenum status = glCheckNamedFramebufferStatus(this->frameBuffer, GL_FRAMEBUFFER);
        FISH_ASSERTF(status == GL_FRAMEBUFFER_COMPLETE, "Framebuffer status not FRAMEBUFFER_COMPLETE: {}", status);
    }

    GLuint Screen::getColorBuffer() { return this->colorBuffer; }
    GLuint Screen::getDepthBuffer() { return this->depthBuffer; }
    GLuint Screen::getFrameBuffer() { return this->frameBuffer; }

    void Screen::clearScreen()
    {
        int width;
        int height;
        glfwGetWindowSize(window, &width, &height);

        glBlitNamedFramebuffer
        (
            this->frameBuffer, 0,
            0, 0, width, height,
            0, 0, width, height,
            GL_COLOR_BUFFER_BIT, GL_LINEAR
        );

        glBindFramebuffer(GL_FRAMEBUFFER, this->frameBuffer);
        glClearDepth(0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

}
