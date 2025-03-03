#version 330 core

in vec2 TexCoord;                       // 從頂點著色器傳遞的紋理坐標
in vec3 FragPos;                        // 世界坐標
in vec3 Normal;                         // 法向量

out vec4 FragColor;                     // 輸出顏色

uniform sampler2D diffuseTexture;       // 植物的顏色紋理
uniform vec3 lightPos;                  // 光源位置
uniform vec3 viewPos;                   // 相機位置
uniform vec3 lightColor;                // 光源顏色

void main() {
    // 紋理顏色
    vec3 textureColor = texture(diffuseTexture, TexCoord).rgb;

    // 計算光照
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 視角方向
    vec3 viewDir = normalize(viewPos - FragPos);

    // 高光反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = spec * lightColor;

    // 結合光照顏色和紋理顏色
    vec3 resultColor = (diffuse + specular) * textureColor;

    // 設置片段顏色
    FragColor = vec4(resultColor, 1.0);
}
