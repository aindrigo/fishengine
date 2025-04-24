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

layout(std140, binding = 1) uniform lightSSBO {
    PointLight pointLights[256];
};


void main()
{
    const vec4 albedoSpec = texture(gAlbedoSpec, fragTexCoords);
    const vec3 fragPos = texture(gPosition, fragTexCoords).rgb;
    const vec3 normal = texture(gNormal, fragTexCoords).rgb;
    const vec3 albedo = albedoSpec.rgb;

    ObjectData obj;
    obj.fragPos = fragPos;
    obj.normal = normal;
    obj.viewPos = viewPos;

    Material mat;
    mat.albedo = albedo;
    mat.roughness = 0.5f;
    mat.metallic = 1.0f;

    LightCalculationData calcData;
    calcData.N = obj.normal;
    calcData.V = normalize(obj.viewPos - obj.fragPos);

    vec3 lightColor = albedo * 0.1;

    if (dirLight.enabled) {
        lightColor += calcDirLight(dirLight, calcData, obj, mat);
    }

    for (uint i = 0; i < pointLights.length(); i++) {
        PointLight light = pointLights[i];
        if (light.enabled == 0)
            continue;
        
        lightColor += calcPointLight(light, calcData, obj, mat);
    }

    lightColor /= lightColor + vec3(1.0);

    vec3 result = lightColor;
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(lightColor, 1.0);
} 