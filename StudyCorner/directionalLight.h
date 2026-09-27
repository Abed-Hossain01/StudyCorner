// directionalLight.h
// A directional light source (e.g. sunlight): it has a DIRECTION but no
// position, and (per the lecture slides) no attenuation -- the source is
// treated as being "at infinity", so its rays are effectively parallel
// and equally strong everywhere in the scene.

#ifndef directionalLight_h
#define directionalLight_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"

class DirectionalLight {
public:
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    DirectionalLight(
        float dirX, float dirY, float dirZ,
        float ambR, float ambG, float ambB,
        float diffR, float diffG, float diffB,
        float specR, float specG, float specB)
    {
        direction = glm::normalize(glm::vec3(dirX, dirY, dirZ));
        ambient = glm::vec3(ambR, ambG, ambB);
        diffuse = glm::vec3(diffR, diffG, diffB);
        specular = glm::vec3(specR, specG, specB);
    }

    // sends this light's data to the shader as "dirLight.<field>"
    void setUpDirectionalLight(Shader& shader)
    {
        shader.use();
        shader.setVec3("dirLight.direction", direction);
        shader.setVec3("dirLight.ambient", isOn ? ambient : glm::vec3(0.0f));
        shader.setVec3("dirLight.diffuse", isOn ? diffuse : glm::vec3(0.0f));
        shader.setVec3("dirLight.specular", isOn ? specular : glm::vec3(0.0f));
    }

    void turnOn() { isOn = true; }
    void turnOff() { isOn = false; }
    bool isLightOn() { return isOn; }

private:
    bool isOn = true;
};

#endif
