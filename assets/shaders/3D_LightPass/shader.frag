#version 430 core
out vec4 FragColor;

#include <lights.glsl>
#include <math.glsl>


in vec2 fragTexCoords;

uniform DirectionalLight dirLight;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 viewPos;


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