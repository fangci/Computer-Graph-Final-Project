#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 Projection;
uniform float time;
uniform sampler2D displacementMap;
uniform float amplitude;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {
    vec3 modifiedPos = aPos;
    float height = texture(displacementMap, aTexCoord).r * amplitude;

    modifiedPos.y += height;

    FragPos = vec3(ModelMatrix * vec4(modifiedPos, 1.0));
    Normal = mat3(transpose(inverse(ModelMatrix))) * aNormal;
    TexCoord = aTexCoord;

    gl_Position = Projection * ViewMatrix * vec4(FragPos, 1.0);
}
