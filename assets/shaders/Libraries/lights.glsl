struct PointLight
{
    vec4 position;
    vec4 color;
    vec4 clqi;
    int enabled;
};

struct DirectionalLight
{
    vec3 direction;
    vec4 color;
    bool enabled;
};

struct Material
{
    float metallic;
    float roughness;
};

layout(std140, binding = 1) uniform lightSSBO {
    PointLight pointLights[256];
};

vec3 calcPointLight(PointLight light, vec3 normal, vec3 albedo, vec3 fragPos, vec3 viewDir, Material material)
{
    vec3 lightDir = normalize(light.position.xyz - fragPos);

    return vec3(0.0);
}

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 albedo, vec3 viewDir, Material material)
{
    vec3 lightDir = normalize(-light.direction);
    return vec3(0.0);
}
