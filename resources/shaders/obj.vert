#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexcoord;
layout (location = 2) in vec3 aNormal;

uniform mat4 uMm = mat4(1.0);
uniform mat3 uMm_normal;
uniform mat4 uVm = mat4(1.0);
uniform mat4 uPm = mat4(1.0);

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

void main()
{
    gl_Position = uPm * uVm * uMm * vec4(aPos, 1.0f);
    FragPos = vec3(uMm * vec4(aPos, 1.0));
    TexCoords = aTexcoord;

    Normal = uMm_normal * aNormal;
}
