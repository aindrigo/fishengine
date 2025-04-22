#version 430 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

uniform sampler2D diffuseMap;

in vec3 fragPos;
in vec3 fragNormal;
in vec2 fragTexCoords;

void main()
{
    gPosition = fragPos;
    gNormal = normalize(fragNormal);

    gAlbedoSpec.rgb = texture(diffuseMap, fragTexCoords).rgb;
    gAlbedoSpec.a = 0.5f;
} 