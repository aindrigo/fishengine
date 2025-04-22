#version 430 core
out vec4 FragColor;

struct PointLight
{
    vec4 position;
    vec4 color;
    vec4 clq;
};


layout(std430, binding = 2) restrict buffer lightSSBO {
    PointLight pointLights[];
};

in vec2 fragTexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 viewPos;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 albedo, vec3 fragPos, vec3 viewDir, float shininess)
{
    vec3 lightDir = normalize(light.position.xyz - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    float distance = length(light.position.xyz - fragPos);
    float atten = 1.0 / (light.clq.x + light.clq.y * distance + light.clq.z * (distance * distance));

    vec3 ambient = light.color.rgb * vec3(0.1) * albedo;
    vec3 diffuse = light.color.rgb * diff * albedo;
    vec3 specular = light.color.rgb * spec * albedo;

    ambient *= atten;
    diffuse *= atten;
    specular *= atten;
    
    return (ambient + diffuse + specular);
}

void main()
{
    vec3 fragPos = texture(gPosition, fragTexCoords).rgb;
    vec3 normal = texture(gNormal, fragTexCoords).rgb;
    vec4 albedoSpec = texture(gAlbedoSpec, fragTexCoords);

    vec3 albedo = albedoSpec.rgb;

    vec3 lightColor = albedo * 0.1;
    vec3 viewDir = normalize(viewPos - fragPos);

    for (int i = 0; i < pointLights.length(); i++) {
        PointLight light = pointLights[i];

        lightColor += calcPointLight(light, normal, albedo, fragPos, viewDir, albedoSpec.a);
    }

    FragColor = vec4(lightColor, 1.0);
} 