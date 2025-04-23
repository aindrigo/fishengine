#version 430 core
out vec4 FragColor;

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

layout(std140, binding = 1) uniform lightSSBO {
    PointLight pointLights[256];
};

in vec2 fragTexCoords;

uniform DirectionalLight dirLight;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 viewPos;

vec3 calcDiffuse(vec3 lightDir, vec3 normal, vec3 diffuseColor, vec3 lightPos, vec3 lightColor)
{
    float NdotL = dot(normal, lightDir);
    float diff = max(NdotL, 0.0);

    return diff * diffuseColor * lightColor;
}

vec3 calcSpecular(vec3 lightDir, vec3 viewDir, vec3 normal, vec3 specularColor, vec3 lightPos, vec3 lightColor, float shininess)
{
    float NdotL = dot(normal, lightDir);

    if (NdotL < 0.0)
        return vec3(0.0);

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    return spec * specularColor * lightColor;
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 albedo, vec3 fragPos, vec3 viewDir, float shininess)
{
    vec3 lightDir = normalize(light.position.xyz - fragPos);

    float distance = length(light.position.xyz - fragPos);
    float atten = light.clqi.w / (light.clqi.x + light.clqi.y * distance + light.clqi.z * (distance * distance));

    vec3 diffuse = calcDiffuse(lightDir, normal, albedo, light.position.xyz, light.color.rgb);
    vec3 specular = calcSpecular(lightDir, viewDir, normal, vec3(1.0), light.position.xyz, light.color.rgb, shininess * 2.0f);

    vec3 col = (diffuse + specular) * atten;

    return col;
}

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 albedo, vec3 viewDir, float shininess)
{
    vec3 lightDir = normalize(-light.direction);
    vec3 diffuse = calcDiffuse(lightDir, normal, albedo, vec3(0.0), light.color.rgb);
    vec3 specular = calcSpecular(lightDir, viewDir, normal, vec3(1.0), vec3(0.0), light.color.rgb, shininess * 2.0f);

    vec3 col = diffuse + specular;
    return col;
}

void main()
{
    const vec4 albedoSpec = texture(gAlbedoSpec, fragTexCoords);
    const vec3 fragPos = texture(gPosition, fragTexCoords).rgb;
    const vec3 normal = texture(gNormal, fragTexCoords).rgb;

    const vec3 albedo = albedoSpec.rgb;
    const vec3 viewDir = normalize(viewPos - fragPos);

    vec3 lightColor = albedo * 0.1;

    if (dirLight.enabled) {
        lightColor += calcDirLight(dirLight, normal, albedo, viewDir, albedoSpec.a);
    }

    for (uint i = 0; i < pointLights.length(); i++) {
        PointLight light = pointLights[i];
        if (light.enabled == 0)
            continue;
        
        lightColor += calcPointLight(light, normal, albedo, fragPos, viewDir, albedoSpec.a);
    }

    FragColor = vec4(lightColor, 1.0);
} 