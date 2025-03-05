#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D lowTex;
uniform sampler2D highTex;
uniform sampler2D Water;
uniform sampler2D displacementMap;

// Directional light
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;


void main() {
    vec4 lowColor = texture(lowTex, TexCoords);
    vec4 highColor = texture(highTex, TexCoords);
    vec4 waterColor = mix(texture(Water, TexCoords), vec4(0.3f, 0.8f, 1.0f, 1.0f), 0.5f) * vec4(lightColor, 1.0f);
    float waterlevel = FragPos.y + texture(displacementMap, TexCoords).r * 10.0f;

    float height = FragPos.y;

    float noise = fract(sin(dot(TexCoords.xy, vec2(12.9898, 78.233))) * 43758.5453);
    noise = mix(-0.2, 0.2, noise);
    height += noise;

    float slopeFactor = dot(normalize(Normal), vec3(0.0, 1.0, 0.0));
    slopeFactor = clamp(slopeFactor, 0.8, 1.5);

    float mixFactor = smoothstep(0.6, 1.5, height);
    if(height<2.5){
        mixFactor = mixFactor * slopeFactor;
    }

    vec4 baseColor = mix(lowColor, highColor, mixFactor);

    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(-lightDir);

    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 ambient = ambientColor;

    vec3 result = (ambient + diffuse) * baseColor.rgb;

    FragColor = vec4(result, baseColor.a);

    float lightness = (dot(vec3(0.0, 1.0, 0.0), lightDir) + 1) / 2 + 0.5;
    FragColor = vec4(FragColor.rgb * lightness, FragColor.a);

    if(waterlevel<0.1){
        FragColor = mix(FragColor, waterColor, 0.5);
    }
}
