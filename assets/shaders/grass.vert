#version 330 core

layout(location = 0) in vec3 position;    // ���I��m
layout(location = 1) in vec3 normal;      // �k�V�q
layout(location = 2) in vec2 texcoord;    // ���z����

out vec2 TexCoord;                       // �ǻ�����q�ۦ⾹�����z����
out vec3 FragPos;                        // �@�ɧ���
out vec3 Normal;                         // �k�V�q

uniform mat4 ModelMatrix;                // �ҫ��x�}
uniform mat4 ViewMatrix;                 // ���ϯx�}
uniform mat4 Projection;                 // ��v�x�}
uniform float time;                      // �ʵe�ɶ�

void main() {
    TexCoord = texcoord;

    // �������j�ĪG�G�b x �M z �b�W���L�\��
    vec3 displacedPosition = position;
    displacedPosition.x += 0.05 * sin(5.0 * position.y + time); // �H�۰����ܤ�
    displacedPosition.z += 0.05 * cos(5.0 * position.y + time);

    // �@�ɧ���
    FragPos = vec3(ModelMatrix * vec4(displacedPosition, 1.0));

    // �k�V�q
    Normal = mat3(transpose(inverse(ModelMatrix))) * normal;

    // �p����ŪŶ���m
    gl_Position = Projection * ViewMatrix * vec4(FragPos, 1.0);
}
