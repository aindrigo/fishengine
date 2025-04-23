#include "fish/glsl.hpp"
#include "fish/services.hpp"
#include <string>

namespace fish
{
    class ShaderManager
    {
    public:
        ShaderManager(Services& services);
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
        GLSLPreProcessor processor;
        std::unordered_map<std::string, ShaderData> shaders;
    };
}