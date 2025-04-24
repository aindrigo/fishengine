#include <math.glsl>

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
    vec3 albedo;
    float metallic;
    float roughness;
};

struct ObjectData
{
    vec3 fragPos;
    vec3 normal;
    vec3 viewDir;
    vec3 viewPos;
};

struct LightCalculationData
{
    vec3 N;
    vec3 V;
};

vec3 calcPbr(vec3 L, vec3 H, float atten, vec3 color, LightCalculationData calcData, Material mat)
{
    vec3 radiance = color * 128.0f * atten;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, mat.albedo, mat.metallic);

    vec3 F = fresnelSchlick(max(dot(H, calcData.V), 0.0), F0);
    float NDF = distributionGGX(calcData.N, H, mat.roughness);
    float G = geometrySmith(calcData.N, calcData.V, L, mat.roughness);

    vec3 numerator = NDF * G * F;
    float denom = 4.0 * max(dot(calcData.N, calcData.V), 0.0) * max(dot(calcData.N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denom;

    vec3 k5 = F;
    vec3 kD = vec3(1.0) - k5;

    kD *= 1.0 - mat.metallic;

    float NdotL = max(dot(calcData.N, L), 0.0);

    return (kD * mat.albedo / PI + specular) * radiance * NdotL;
}

vec3 calcPointLight(PointLight light, LightCalculationData calcData, ObjectData obj, Material mat)
{
    vec3 L = normalize(light.position.xyz - obj.fragPos);
    vec3 H = normalize(calcData.V + L);
    float distance = length(light.position.xyz - obj.fragPos);
    float atten = 1.0 / ((distance * distance));

    return calcPbr(L, H, atten, light.color.rgb, calcData, mat);
}

vec3 calcDirLight(DirectionalLight light, LightCalculationData calcData, ObjectData obj, Material mat)
{
    return calcPbr(light.direction, normalize(calcData.V), 0.1, light.color.rgb, calcData, mat);
}
