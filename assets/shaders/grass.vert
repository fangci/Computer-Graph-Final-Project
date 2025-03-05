#version 330 core

layout(location = 0) in vec3 position;    // 頂點位置
layout(location = 1) in vec3 normal;      // 法向量
layout(location = 2) in vec2 texcoord;    // 紋理坐標

out vec2 TexCoord;                       // 傳遞到片段著色器的紋理坐標
out vec3 FragPos;                        // 世界坐標
out vec3 Normal;                         // 法向量

uniform mat4 ModelMatrix;                // 模型矩陣
uniform mat4 ViewMatrix;                 // 視圖矩陣
uniform mat4 Projection;                 // 投影矩陣
uniform float time;                      // 動畫時間

void main() {
    TexCoord = texcoord;

    // 模擬風吹效果：在 x 和 z 軸上輕微擺動
    vec3 displacedPosition = position;
    displacedPosition.x += 0.05 * sin(5.0 * position.y + time); // 隨著高度變化
    displacedPosition.z += 0.05 * cos(5.0 * position.y + time);

    // 世界坐標
    FragPos = vec3(ModelMatrix * vec4(displacedPosition, 1.0));

    // 法向量
    Normal = mat3(transpose(inverse(ModelMatrix))) * normal;

    // 計算裁剪空間位置
    gl_Position = Projection * ViewMatrix * vec4(FragPos, 1.0);
}
