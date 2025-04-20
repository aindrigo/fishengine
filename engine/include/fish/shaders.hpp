#include "fish/services.hpp"
#include <string>

namespace fish
{
    class ShaderCache
    {
    public:
        ShaderCache(Services& services);
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
        std::unordered_map<std::string, ShaderData> shaders;
    };
}