#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 Tex;
layout (location = 2) in vec3 normal;


out vec2 TexCoord; // Add a output variable for texture coordinates
out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
    FragPos = vec3(model * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal;
    TexCoord = Tex;
    gl_Position = projection * view * vec4(FragPos, 1.0);
    
}
