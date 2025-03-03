#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform sampler2D displacementMap;
uniform vec3 lightColor;
uniform vec3 lightDir;
uniform vec3 ambientColor;
uniform vec3 waterColor;
uniform float time;

out vec4 FragColor;

float rand(vec2 co){
	return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    vec3 perturbedNormal = normalize(Normal + vec3(
        sin(rand(TexCoord + time)) * 0.1,
        0.0,
        cos(rand(TexCoord + time)) * 0.1));
    vec3 lightDirNorm = normalize(lightDir) * dot(Normal, -lightDir);

    float diff = max(dot(perturbedNormal, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 ambient = ambientColor;

    vec3 lighting = (ambient * 0.5 + diffuse * 1.5) / 2;

    vec3 lightPos = lightDir * 75.0;
    vec3 lightDist = FragPos - lightPos;
    float dist = length(lightDist);
    float attenuation = 75.0 / dist * 2;
    lighting += attenuation * lightColor;

    vec3 textureColor = texture(displacementMap, TexCoord).rgb;
    vec3 imgColor = texture(ourTexture, TexCoord).rgb;
    vec3 blendWater = mix(waterColor, lightColor, 0.5);
    vec3 blend = mix(imgColor, blendWater, 0.8);
    float displacement = texture(displacementMap, TexCoord).r * 300.0;
    if (displacement < 1.2) {
		displacement = 0.0;
	}
    else displacement *= 0.75;
    vec3 displacementColor = displacement * lightColor;
    vec3 textblend = mix(textureColor, blend, 0.5) * lighting;
    vec3 finalColor = mix(textblend, displacementColor, 0.4);

    float lightness = (dot(Normal, lightDir) + 1) / 2 + 0.5;
    finalColor = lightness * finalColor;

    float alpha = clamp(dot(lightDirNorm, perturbedNormal) * 0.5 + 0.5, 0.8, 0.9);
    FragColor = vec4(finalColor, alpha);
}
