#version 410 core
// Světla https://learnopengl.com/Lighting/Multiple-lights

// Input
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

// Output
out vec4 FragColor;

// Lights definition
struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float transparency;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // Attenuation (útlum)
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // Attenuation (útlum)
    float constant;
    float linear;
    float quadratic;
};

// Pozice kamery
uniform vec3 viewPos;

// Lights
uniform DirLight dirLight;
#define NR_POINT_LIGHTS 3
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

//Textures
uniform sampler2D tex0;  // Textura
uniform Material material;



vec3 CalcDirLight(DirLight light, vec3 norm, vec3 viewDir, vec3 textureColor);
vec3 CalcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 textureColor);
vec3 CalcSpotLight(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 textureColor);


void main()
{
    // Výpočet barvy z textury
    vec3 textureColor = texture(tex0, TexCoords).rgb;

    //
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // define an output color value
    vec3 light = vec3(0.0);

    // Výpočet - Directional light
    light += CalcDirLight(dirLight, norm, viewDir, textureColor);

    // Výpočet - Pont light
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        light += CalcPointLight(pointLights[i], norm, FragPos, viewDir, textureColor);

    // Výpočet - Spot light
    light += CalcSpotLight(spotLight, norm, FragPos, viewDir, textureColor);

    // Výsledná barva fragmentu
    FragColor = vec4(light, material.transparency);
}

vec3 CalcDirLight(DirLight light, vec3 norm, vec3 viewDir, vec3 textureColor)
{
    // diff - diffuse shading
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);

    // spec - specular shading
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // Ambientní osvětlení + Difúzní osvětlení + Speculární osvětlení
    vec3 ambient = light.ambient * textureColor * material.ambient;
    vec3 diffuse = light.diffuse * diff * textureColor; //* material.diffuse;
    vec3 specular = light.specular * spec * textureColor; //* material.specular;

    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 textureColor)
{
    // diff - diffuse shading
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    // spec - specular shading
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Combine results
    vec3 ambient = light.ambient * textureColor * material.ambient;
    vec3 diffuse = light.diffuse * diff * textureColor; //* material.diffuse;
    vec3 specular = light.specular * spec * textureColor; //* material.specular;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 textureColor)
{
    // diff - diffuse shading
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    // spec - specular shading
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // Combine results
    vec3 ambient = light.ambient * textureColor * material.ambient;
    vec3 diffuse = light.diffuse * diff * textureColor; //* material.diffuse;
    vec3 specular = light.specular * spec * textureColor; //* material.specular;

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}
