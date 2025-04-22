#version 330 core
layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inTexCoords;

out vec2 fragTexCoords;

void main()
{
    fragTexCoords = inTexCoords;
    gl_Position = vec4(inPos, 1.0, 1.0);
}