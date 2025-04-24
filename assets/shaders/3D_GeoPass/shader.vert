#version 330 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBitangent;
layout (location = 4) in vec2 inTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;
uniform mat4 mvp;
uniform bool hasNormalMap;

out vec3 fragPos;
out vec3 modelPosition;
out vec3 fragNormal;
out vec2 fragTexCoords;
out mat3 TBN;
void main()
{
    fragPos = (model * vec4(inPos, 1.0)).xyz;
    fragTexCoords = inTexCoords;
    fragNormal = mat3(transpose(inverse(model))) * inNormal;

    if (hasNormalMap) {
        TBN = transpose(mat3(
            normalize(vec3(model * vec4(inTangent, 0.0))),
            normalize(vec3(model * vec4(inBitangent, 0.0))),
            normalize(vec3(model * vec4(fragNormal, 0.0)))
        ));
    }

    gl_Position = mvp * vec4(inPos, 1.0);
}