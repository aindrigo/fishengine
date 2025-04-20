#version 330 core
out vec4 FragColor;

in vec3 fragNormal;
in vec2 fragTexCoords;

uniform sampler2D diffuseMap;

void main()
{
    FragColor = texture(diffuseMap, fragTexCoords);
    //FragColor.rgb = pow(FragColor.rgb, vec3(0.454545455));
} 