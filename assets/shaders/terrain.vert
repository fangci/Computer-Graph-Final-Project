#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

// Uniforms
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 Projection;

void main() {
    gl_Position = Projection * ViewMatrix * ModelMatrix * vec4(aPos, 1.0);

    FragPos = vec3(ModelMatrix * vec4(aPos, 1.0));

    Normal = mat3(transpose(inverse(ModelMatrix))) * aNormal;

    TexCoords = aTexCoords;
}