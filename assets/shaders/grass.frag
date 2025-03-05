#version 330 core

in vec2 TexCoord;                       // �q���I�ۦ⾹�ǻ������z����
in vec3 FragPos;                        // �@�ɧ���
in vec3 Normal;                         // �k�V�q

out vec4 FragColor;                     // ��X�C��

uniform sampler2D diffuseTexture;       // �Ӫ����C�⯾�z
uniform vec3 lightPos;                  // ������m
uniform vec3 viewPos;                   // �۾���m
uniform vec3 lightColor;                // �����C��

void main() {
    // ���z�C��
    vec3 textureColor = texture(diffuseTexture, TexCoord).rgb;

    // �p�����
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    // ���Ϯg
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // ������V
    vec3 viewDir = normalize(viewPos - FragPos);

    // �����Ϯg
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * lightColor;

    // ���X�����C��M���z�C��
    vec3 resultColor = (diffuse + specular) * textureColor;

    // �]�m���q�C��
    FragColor = vec4(resultColor, 1.0);
}
