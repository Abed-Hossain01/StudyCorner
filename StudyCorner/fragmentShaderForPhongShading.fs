#version 330 core
out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct PointLight {
    vec3 position;

    float k_c;  // attenuation factors
    float k_l;
    float k_q;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct DirectionalLight {
    vec3 direction;   // no position, no attenuation -- "light from infinity"

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform PointLight pointLight;
uniform DirectionalLight dirLight;
uniform Material material;

// function prototypes
vec3 CalcPointLight(PointLight light, vec3 N, vec3 fragPos, vec3 V);
vec3 CalcDirLight(DirectionalLight light, vec3 N, vec3 V);

void main()
{
    // properties
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);
    result += CalcDirLight(dirLight, N, V);
    result += CalcPointLight(pointLight, N, FragPos, V);

    FragColor = vec4(result, 1.0);
}

// calculates the color contribution from the point light (the lamp bulb)
vec3 CalcPointLight(PointLight light, vec3 N, vec3 fragPos, vec3 V)
{
    vec3 L = normalize(light.position - fragPos);
    vec3 R = reflect(-L, N);

    vec3 K_A = material.ambient;
    vec3 K_D = material.diffuse;
    vec3 K_S = material.specular;

    // attenuation: light gets weaker with distance
    float d = length(light.position - fragPos);
    float attenuation = 1.0 / (light.k_c + light.k_l * d + light.k_q * (d * d));

    vec3 ambient = K_A * light.ambient;
    vec3 diffuse = K_D * max(dot(N, L), 0.0) * light.diffuse;
    vec3 specular = K_S * pow(max(dot(V, R), 0.0), material.shininess) * light.specular;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

// calculates the color contribution from the directional light (the "sun")
vec3 CalcDirLight(DirectionalLight light, vec3 N, vec3 V)
{
    // light travels FROM light.direction, so the direction TOWARDS the light is the negative
    vec3 L = normalize(-light.direction);
    vec3 R = reflect(-L, N);

    vec3 K_A = material.ambient;
    vec3 K_D = material.diffuse;
    vec3 K_S = material.specular;

    vec3 ambient = K_A * light.ambient;
    vec3 diffuse = K_D * max(dot(N, L), 0.0) * light.diffuse;
    vec3 specular = K_S * pow(max(dot(V, R), 0.0), material.shininess) * light.specular;

    // no attenuation here -- a directional light does not weaken with distance
    return (ambient + diffuse + specular);
}
