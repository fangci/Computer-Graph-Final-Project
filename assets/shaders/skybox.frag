#version 330 core

in vec3 TexCoords;

out vec4 FragColor;

uniform samplerCube skybox;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 horizonColor;
uniform vec3 zenithColor;

void main() {
    float intensity = max(dot(normalize(TexCoords), normalize(lightDir)), 0.0);
    intensity = pow(intensity, 1.5);

    vec3 dynamicSkyColor = mix(horizonColor, zenithColor, TexCoords.y * 0.5 + 0.5);

    dynamicSkyColor = mix(dynamicSkyColor, lightColor, intensity);

    vec4 skyboxColor = texture(skybox, TexCoords);
    FragColor = vec4(dynamicSkyColor * skyboxColor.rgb, skyboxColor.a);
}
