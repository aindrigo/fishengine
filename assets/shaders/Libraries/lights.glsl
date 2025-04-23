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

vec3 calcDiffuse(vec3 lightDir, vec3 normal, vec3 diffuseColor, vec3 lightPos, vec3 lightColor)
{
    float NdotL = dot(normal, lightDir);
    float diff = max(NdotL, 0.0);

    return diff * diffuseColor * lightColor;
}

vec3 calcSpecular(vec3 lightDir, vec3 viewDir, vec3 normal, vec3 specularColor, vec3 lightPos, vec3 lightColor, float shininess)
{
    float NdotL = dot(normal, lightDir);

    if (NdotL <= 0.0)
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
