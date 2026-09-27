// pointLight.h
// A single point light source (e.g. a lamp bulb).
// Same idea as taught in class (position + ambient/diffuse/specular +
// attenuation factors k_c, k_l, k_q), just simplified to ONE light
// instead of four, since our scene only needs one.

#ifndef pointLight_h
#define pointLight_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"

class PointLight {
public:
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float k_c;   // constant attenuation term
    float k_l;   // linear attenuation term
    float k_q;   // quadratic attenuation term

    PointLight(
        float posX, float posY, float posZ,
        float ambR, float ambG, float ambB,
        float diffR, float diffG, float diffB,
        float specR, float specG, float specB,
        float constant, float linear, float quadratic)
    {
        position = glm::vec3(posX, posY, posZ);
        ambient = glm::vec3(ambR, ambG, ambB);
        diffuse = glm::vec3(diffR, diffG, diffB);
        specular = glm::vec3(specR, specG, specB);
        k_c = constant;
        k_l = linear;
        k_q = quadratic;
    }

    // sends this light's data to the shader as "pointLight.<field>"
    void setUpPointLight(Shader& shader)
    {
        shader.use();
        shader.setVec3("pointLight.position", position);
        shader.setVec3("pointLight.ambient", isOn ? ambient : glm::vec3(0.0f));
        shader.setVec3("pointLight.diffuse", isOn ? diffuse : glm::vec3(0.0f));
        shader.setVec3("pointLight.specular", isOn ? specular : glm::vec3(0.0f));
        shader.setFloat("pointLight.k_c", k_c);
        shader.setFloat("pointLight.k_l", k_l);
        shader.setFloat("pointLight.k_q", k_q);
    }

    void turnOn() { isOn = true; }
    void turnOff() { isOn = false; }
    bool isLightOn() { return isOn; }

private:
    bool isOn = true;
};

#endif
