#version 430

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform sampler2D ourTexture;

uniform vec3 viewPos;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

struct DirectionLight
{
    int enable;
    vec3 direction;
    vec3 lightColor;
};

struct PointLight {
    int enable;
    vec3 position;  
    vec3 lightColor;

    float constant;
    float linear;
    float quadratic;
};

struct Spotlight {
    int enable;
    vec3 position;
    vec3 direction;
    vec3 lightColor;
    float cutOff;

    // Paramters for attenuation formula
    float constant;
    float linear;
    float quadratic;      
}; 

uniform Material material;
uniform DirectionLight dl;
uniform PointLight pl;
uniform Spotlight sl;


void main() {
    
    vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);
    // ambient= La * Ka
    // diffuse = Ld * Kd * max(dot(N, L), 0)
    // specular = Ls * Ks * max(dot(V, R), 0) = Ks * pow(max(dot(N, H), 0), shininess)

	// Direction light
    vec3 dl_amibent = dl.enable * material.ambient * dl.lightColor;
	vec3 dl_lightDir = normalize(-dl.direction);
	float dl_diff = max(dot(norm, dl_lightDir), 0.0);
	vec3 dl_diffuse = dl.enable * dl_diff * dl.lightColor * material.diffuse;
    vec3 dl_specular = dl.enable * material.specular * dl.lightColor * max(dot(viewDir, reflect(-dl_lightDir, norm)), 0.0);
	result += dl_amibent + dl_diffuse + dl_specular;

    // Point light
    vec3 pl_ambient = pl.enable * material.ambient * pl.lightColor;
    vec3 pl_lightDir = normalize(pl.position - FragPos);
    float pl_diff = max(dot(norm, pl_lightDir), 0.0);
    vec3 pl_diffuse = pl.enable * pl_diff * pl.lightColor * material.diffuse;
    vec3 pl_specular = pl.enable * material.specular * pl.lightColor * max(dot(viewDir, reflect(-pl_lightDir, norm)), 0.0);
    float pl_distance = length(pl.position - FragPos);
    float pl_attenuation = 1.0 / (pl.constant + pl.linear * pl_distance + pl.quadratic * (pl_distance * pl_distance));
    result += pl_ambient + pl_attenuation * (pl_diffuse + pl_specular);

    // Spotlight
    vec3 sl_ambient = sl.enable * material.ambient * sl.lightColor;
    vec3 sl_lightDir = normalize(sl.position - FragPos);
    float sl_diff = max(dot(norm, sl_lightDir), 0.0);
    vec3 sl_diffuse = sl.enable * sl_diff * sl.lightColor * material.diffuse;
    vec3 sl_halfDir = normalize(sl_lightDir + viewDir);
    vec3 sl_specular = sl.enable * material.specular * pow(max(dot(norm, sl_halfDir), 0.0), material.shininess);
    float sl_distance = length(sl.position - FragPos);
    float sl_attenuation = 1.0 / (sl.constant + sl.linear * sl_distance + sl.quadratic * (sl_distance * sl_distance));
    float theta = dot(sl_lightDir, normalize(-sl.direction)); // angle between light direction and spotlight direction
    if (theta > sl.cutOff) {
        result += sl_ambient + sl_attenuation * (sl_diffuse + sl_specular);
    } else {
		result += sl_ambient;
    }

    vec4 texColor = texture(ourTexture, TexCoord);
    result = result * texColor.rgb;
    color = vec4(result, texColor.a);
    
}
