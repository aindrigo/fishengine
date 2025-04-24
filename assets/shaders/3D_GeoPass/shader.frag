#version 430 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedoSpec;
layout (location = 3) out vec3 gModelPosition;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform bool hasNormalMap;

in vec3 fragPos;
in vec3 modelPosition;
in vec3 fragNormal;
in vec2 fragTexCoords;

void main()
{
    gPosition = fragPos;
    if (hasNormalMap) {
        gNormal = texture(normalMap, fragTexCoords).rgb;
    } else {
        gNormal = normalize(fragNormal);
    }

    gAlbedoSpec.rgb = texture(diffuseMap, fragTexCoords).rgb;
    gModelPosition = modelPosition;
} 