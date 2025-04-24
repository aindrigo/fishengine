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
uniform vec3 position;

out vec3 fragPos;
out vec3 modelPosition;
out vec3 fragNormal;
out vec2 fragTexCoords;

void main()
{
    fragPos = (model * vec4(inPos, 1.0)).xyz;
    fragTexCoords = inTexCoords;
    fragNormal = mat3(transpose(inverse(model))) * inNormal;
    modelPosition = position;
    gl_Position = mvp * vec4(inPos, 1.0);
}