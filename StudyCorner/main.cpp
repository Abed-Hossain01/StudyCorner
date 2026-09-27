// ==========================================================================
//Project: Study Corner
// ==========================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "pointLight.h"
#include "directionalLight.h"

#include <iostream>
#include <cmath>

using namespace std;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);

glm::mat4 boxModel(glm::vec3 pos, glm::vec3 size);
void drawCube(unsigned int& cubeVAO, Shader& shader, glm::mat4 model, float r, float g, float b);

void drawFloor(unsigned int& cubeVAO, Shader& shader);
void drawCeiling(unsigned int& cubeVAO, Shader& shader);
void drawSideWalls(unsigned int& cubeVAO, Shader& shader);
void drawBackWallWithWindow(unsigned int& cubeVAO, Shader& shader);
void drawWindowScenery(unsigned int& cubeVAO, Shader& shader, float rainOffset);
void drawTable(unsigned int& cubeVAO, Shader& shader);
void drawChair(unsigned int& cubeVAO, Shader& shader);
void drawLamp(unsigned int& cubeVAO, Shader& lightingShader, Shader& constantShader);
void drawMonitor(unsigned int& cubeVAO, Shader& lightingShader, Shader& constantShader);
void drawBookshelf(unsigned int& cubeVAO, Shader& shader);
void drawFan(unsigned int& cubeVAO, Shader& shader, float angle);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera (this IS our viewing transformation-WASD)
Camera camera(glm::vec3(0.0f, 1.6f, -5.0f));

float deltaTime = 0.0f;
float lastFrame = 0.0f;


const float roomHalfWidth = 5.0f;   
const float roomBackZ = 4.5f;    
const float roomFrontZ = -5.0f;  
const float roomCenterZ = (roomFrontZ + roomBackZ) / 2.0f; 
const float roomDepth = roomBackZ - roomFrontZ;      
const float roomHeight = 4.0f;   


const float windowWidth = 1.6f;
const float windowHeight = 1.4f;
const float windowBottomY = 1.3f;
const float windowTopY = windowBottomY + windowHeight;

const float tableZ = 1.5f;      
const float legH = 0.70f;      
const float topH = 0.06f;      
const float topD = 0.8f;       

const glm::vec3 lampBasePos(0.40f, legH + topH, tableZ + 0.20f);
const glm::vec3 lampStandPos = lampBasePos + glm::vec3(0.0f, 0.03f, 0.0f);
const glm::vec3 lampBulbPos = lampStandPos + glm::vec3(0.0f, 0.28f, 0.0f);
const glm::vec3 pointLightPos = lampBulbPos + glm::vec3(0.0f, 0.05f, 0.0f); 

const glm::vec3 fanCenter(0.0f, 2.3f, tableZ);

// animation state

float fanRotationAngle = 0.0f;
float fanSpeed = 90.0f; 
bool fanOn = true;

float rainOffset = 0.0f; 
float rainSpeed = 1.0f;  


// the two required lights

PointLight lampLight(
    pointLightPos.x, pointLightPos.y, pointLightPos.z,   // position
    0.05f, 0.05f, 0.03f,     // ambient  (dim warm)
    0.90f, 0.75f, 0.40f,     // diffuse  (warm yellow-orange, like a bulb)
    1.00f, 0.90f, 0.60f,     // specular
    1.0f,     
    0.09f,    
    0.032f    
);

DirectionalLight sunLight(
    -0.3f, -1.0f, 0.4f,      // direction the light travels (down + sideways)
    0.15f, 0.15f, 0.18f,     // ambient  (soft cool tint)
    0.55f, 0.55f, 0.50f,     // diffuse  (pale sunlight)
    0.30f, 0.30f, 0.30f      // specular
);

int main()
{
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Mini Project - Study Corner", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);


    Shader lightingShader("vertexShaderForPhongShading.vs", "fragmentShaderForPhongShading.fs");
    Shader constantShader("vertexShader.vs", "fragmentShader.fs");

    
    // set up one unit cube (position + normal per vertex).
    // every object in the scene (floor, wall, table, lamp, fan) 
 
    float cube_vertices[] = {
        // positions          // normals
        0.0f, 0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        1.0f, 1.0f, 0.0f,   0.0f, 0.0f, -1.0f,
        0.0f, 1.0f, 0.0f,   0.0f, 0.0f, -1.0f,

        1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f,   1.0f, 0.0f, 0.0f,

        0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,   0.0f, 0.0f, 1.0f,

        0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,

        1.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,   0.0f, -1.0f, 0.0f
    };
    unsigned int cube_indices[] = {
        0, 3, 2,   2, 1, 0,
        4, 5, 7,   7, 6, 4,
        8, 9, 10,  10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    unsigned int cubeVAO, cubeVBO, cubeEBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    
    while (!glfwWindowShouldClose(window))
    {
    
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        if (fanOn)
        {
            fanRotationAngle += fanSpeed * deltaTime;
            if (fanRotationAngle > 360.0f)
                fanRotationAngle -= 360.0f;
        }

       
        rainOffset += rainSpeed * deltaTime;
        if (rainOffset > windowHeight)
            rainOffset -= windowHeight;

        
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        lightingShader.use();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setVec3("viewPos", camera.Position);

        lampLight.setUpPointLight(lightingShader);
        sunLight.setUpDirectionalLight(lightingShader);

        
        constantShader.use();
        constantShader.setMat4("projection", projection);
        constantShader.setMat4("view", view);

        drawFloor(cubeVAO, lightingShader);
        drawCeiling(cubeVAO, lightingShader);
        drawSideWalls(cubeVAO, lightingShader);
        drawBackWallWithWindow(cubeVAO, lightingShader);
        drawWindowScenery(cubeVAO, lightingShader, rainOffset);
        drawTable(cubeVAO, lightingShader);
        drawChair(cubeVAO, lightingShader);
        drawLamp(cubeVAO, lightingShader, constantShader);
        drawMonitor(cubeVAO, lightingShader, constantShader);
        drawBookshelf(cubeVAO, lightingShader);
        drawFan(cubeVAO, lightingShader, fanRotationAngle);

        // glfw: swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &cubeEBO);

    glfwTerminate();
    return 0;
}

glm::mat4 boxModel(glm::vec3 pos, glm::vec3 size)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);             
    model = glm::scale(model, size);                  
    model = glm::translate(model, glm::vec3(-0.5f, 0.0f, -0.5f));
    return model;
}

void drawCube(unsigned int& cubeVAO, Shader& shader, glm::mat4 model, float r, float g, float b)
{
    shader.use();
    shader.setVec3("material.ambient", glm::vec3(r, g, b));
    shader.setVec3("material.diffuse", glm::vec3(r, g, b));
    shader.setVec3("material.specular", glm::vec3(0.4f, 0.4f, 0.4f));
    shader.setFloat("material.shininess", 32.0f);
    shader.setMat4("model", model);

    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void drawFloor(unsigned int& cubeVAO, Shader& shader)
{
    glm::mat4 model = boxModel(glm::vec3(0.0f, 0.0f, roomCenterZ), glm::vec3(roomHalfWidth * 2.0f, 0.05f, roomDepth));
    drawCube(cubeVAO, shader, model, 0.55f, 0.45f, 0.35f); // tan floor
}

void drawCeiling(unsigned int& cubeVAO, Shader& shader)
{
    glm::mat4 model = boxModel(glm::vec3(0.0f, roomHeight, roomCenterZ), glm::vec3(roomHalfWidth * 2.0f, 0.05f, roomDepth));
    drawCube(cubeVAO, shader, model, 0.85f, 0.85f, 0.82f); // off-white ceiling
}

void drawSideWalls(unsigned int& cubeVAO, Shader& shader)
{
    glm::vec3 wallColor(0.80f, 0.78f, 0.70f); // warm light gray

    // left wall (x = -roomHalfWidth)
    drawCube(cubeVAO, shader, boxModel(glm::vec3(-roomHalfWidth, 0.0f, roomCenterZ), glm::vec3(0.05f, roomHeight, roomDepth)), wallColor.r, wallColor.g, wallColor.b);

    // right wall (x = +roomHalfWidth)
    drawCube(cubeVAO, shader, boxModel(glm::vec3(roomHalfWidth, 0.0f, roomCenterZ), glm::vec3(0.05f, roomHeight, roomDepth)), wallColor.r, wallColor.g, wallColor.b);
}

void drawBackWallWithWindow(unsigned int& cubeVAO, Shader& shader)
{
    float wallZ = roomBackZ;
    float wallW = roomHalfWidth * 2.0f;
    float wallT = 0.05f;
    glm::vec3 wallColor(0.75f, 0.85f, 0.95f);

    // window opening, centered on the desk (desk is at x = 0)
    float winHalfW = windowWidth / 2.0f;    // 0.8

    // band below the window (full width)
    drawCube(cubeVAO, shader, boxModel(glm::vec3(0.0f, 0.0f, wallZ), glm::vec3(wallW, windowBottomY, wallT)), wallColor.r, wallColor.g, wallColor.b);

    // band above the window (full width)
    drawCube(cubeVAO, shader, boxModel(glm::vec3(0.0f, windowTopY, wallZ), glm::vec3(wallW, roomHeight - windowTopY, wallT)), wallColor.r, wallColor.g, wallColor.b);

    // band to the left of the window
    float sideBandW = wallW / 2.0f - winHalfW;
    float leftBandX = -winHalfW - sideBandW / 2.0f;
    drawCube(cubeVAO, shader, boxModel(glm::vec3(leftBandX, windowBottomY, wallZ), glm::vec3(sideBandW, windowHeight, wallT)), wallColor.r, wallColor.g, wallColor.b);

    // band to the right of the window
    float rightBandX = winHalfW + sideBandW / 2.0f;
    drawCube(cubeVAO, shader, boxModel(glm::vec3(rightBandX, windowBottomY, wallZ), glm::vec3(sideBandW, windowHeight, wallT)), wallColor.r, wallColor.g, wallColor.b);

    // thin wooden frame trim around the window
    glm::vec3 frameColor(0.35f, 0.25f, 0.15f);
    float frameT = 0.06f;
    drawCube(cubeVAO, shader, boxModel(glm::vec3(0.0f, windowTopY, wallZ), glm::vec3(windowWidth + frameT * 2.0f, frameT, wallT + 0.02f)), frameColor.r, frameColor.g, frameColor.b);
    drawCube(cubeVAO, shader, boxModel(glm::vec3(0.0f, windowBottomY - frameT, wallZ), glm::vec3(windowWidth + frameT * 2.0f, frameT, wallT + 0.02f)), frameColor.r, frameColor.g, frameColor.b);
    drawCube(cubeVAO, shader, boxModel(glm::vec3(-winHalfW - frameT / 2.0f, windowBottomY, wallZ), glm::vec3(frameT, windowHeight, wallT + 0.02f)), frameColor.r, frameColor.g, frameColor.b);
    drawCube(cubeVAO, shader, boxModel(glm::vec3(winHalfW + frameT / 2.0f, windowBottomY, wallZ), glm::vec3(frameT, windowHeight, wallT + 0.02f)), frameColor.r, frameColor.g, frameColor.b);
}


void drawWindowScenery(unsigned int& cubeVAO, Shader& shader, float rainOffset)
{
    float skyZ = roomBackZ + 0.6f;   
    float rainZ = roomBackZ + 0.25f; 

  
    drawCube(cubeVAO, shader,
        boxModel(glm::vec3(0.0f, windowBottomY - 0.2f, skyZ), glm::vec3(windowWidth + 0.4f, windowHeight + 0.4f, 0.05f)),
        0.55f, 0.58f, 0.62f);

   
    drawCube(cubeVAO, shader,
        boxModel(glm::vec3(0.0f, windowBottomY - 0.2f, skyZ - 0.03f), glm::vec3(windowWidth + 0.4f, 0.35f, 0.05f)),
        0.20f, 0.28f, 0.20f);

    
    const int numDrops = 9;
    float dropX[numDrops] = { -0.65f, -0.5f, -0.35f, -0.2f, -0.05f, 0.1f, 0.25f, 0.4f, 0.55f };
    float dropPhase[numDrops] = { 0.00f, 0.65f, 0.20f, 0.90f, 0.45f, 0.10f, 0.75f, 0.30f, 0.55f };

    for (int i = 0; i < numDrops; i++)
    {
        float y = windowTopY - fmodf(rainOffset + dropPhase[i] * windowHeight, windowHeight);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(dropX[i], y, rainZ));
        model = glm::rotate(model, glm::radians(12.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // 3D TRANSFORMATION: rotation (wind-blown tilt)
        model = glm::scale(model, glm::vec3(0.015f, 0.16f, 0.015f));                  // 3D TRANSFORMATION: scaling (thin streak)
        model = glm::translate(model, glm::vec3(-0.5f, -0.5f, -0.5f));                // center the streak on its own origin

        drawCube(cubeVAO, shader, model, 0.75f, 0.85f, 0.95f);
    }
}

void drawTable(unsigned int& cubeVAO, Shader& shader)
{
    float topW = 1.2f; 
    float legOffX = topW / 2.0f - 0.05f;
    float legOffZ = topD / 2.0f - 0.05f;

    glm::vec3 legColor(0.30f, 0.18f, 0.09f);
    glm::vec3 topColor(0.45f, 0.29f, 0.15f);

    // 4 legs (translation moves each one to its own corner)
    drawCube(cubeVAO, shader, boxModel(glm::vec3(legOffX, 0.0f, tableZ + legOffZ), glm::vec3(0.06f, legH, 0.06f)), legColor.r, legColor.g, legColor.b);
    drawCube(cubeVAO, shader, boxModel(glm::vec3(-legOffX, 0.0f, tableZ + legOffZ), glm::vec3(0.06f, legH, 0.06f)), legColor.r, legColor.g, legColor.b);
    drawCube(cubeVAO, shader, boxModel(glm::vec3(legOffX, 0.0f, tableZ - legOffZ), glm::vec3(0.06f, legH, 0.06f)), legColor.r, legColor.g, legColor.b);
    drawCube(cubeVAO, shader, boxModel(glm::vec3(-legOffX, 0.0f, tableZ - legOffZ), glm::vec3(0.06f, legH, 0.06f)), legColor.r, legColor.g, legColor.b);

    // table top
    drawCube(cubeVAO, shader, boxModel(glm::vec3(0.0f, legH, tableZ), glm::vec3(topW, topH, topD)), topColor.r, topColor.g, topColor.b);
}


void drawChair(unsigned int& cubeVAO, Shader& shader)
{
    float seatW = 0.5f, seatD = 0.5f, seatThick = 0.05f;
    float seatH = 0.45f;             // seat sits a little lower than the table top
    float legOffX = seatW / 2.0f - 0.04f;
    float legOffZ = seatD / 2.0f - 0.04f;
    float chairX = 0.0f;                                   // centered with the table
    float chairZ = tableZ - topD / 2.0f - 0.45f;            // out in front of the table's near edge

    glm::vec3 legColor(0.30f, 0.18f, 0.09f);   // same wood tone as the table legs
    glm::vec3 seatColor(0.25f, 0.35f, 0.55f);  // simple blue "fabric" seat/back

    // 4 legs
    drawCube(cubeVAO, shader, boxModel(glm::vec3(chairX + legOffX, 0.0f, chairZ + legOffZ), glm::vec3(0.05f, seatH, 0.05f)), legColor.r, legColor.g, legColor.b);
    drawCube(cubeVAO, shader, boxModel(glm::vec3(chairX - legOffX, 0.0f, chairZ + legOffZ), glm::vec3(0.05f, seatH, 0.05f)), legColor.r, legColor.g, legColor.b);
    drawCube(cubeVAO, shader, boxModel(glm::vec3(chairX + legOffX, 0.0f, chairZ - legOffZ), glm::vec3(0.05f, seatH, 0.05f)), legColor.r, legColor.g, legColor.b);
    drawCube(cubeVAO, shader, boxModel(glm::vec3(chairX - legOffX, 0.0f, chairZ - legOffZ), glm::vec3(0.05f, seatH, 0.05f)), legColor.r, legColor.g, legColor.b);

    // seat
    drawCube(cubeVAO, shader, boxModel(glm::vec3(chairX, seatH, chairZ), glm::vec3(seatW, seatThick, seatD)), seatColor.r, seatColor.g, seatColor.b);

   
    float backH = 0.5f, backThick = 0.05f;
    glm::vec3 backPos(chairX, seatH + seatThick, chairZ - seatD / 2.0f + backThick / 2.0f);
    drawCube(cubeVAO, shader, boxModel(backPos, glm::vec3(seatW, backH, backThick)), seatColor.r, seatColor.g, seatColor.b);
}

void drawLamp(unsigned int& cubeVAO, Shader& lightingShader, Shader& constantShader)
{
    drawCube(cubeVAO, lightingShader, boxModel(lampBasePos, glm::vec3(0.14f, 0.03f, 0.14f)), 0.20f, 0.20f, 0.20f);
    drawCube(cubeVAO, lightingShader, boxModel(lampStandPos, glm::vec3(0.03f, 0.28f, 0.03f)), 0.20f, 0.20f, 0.20f);

    glm::mat4 bulbModel = boxModel(lampBulbPos, glm::vec3(0.10f));
    constantShader.use();
    constantShader.setMat4("model", bulbModel);
    constantShader.setVec3("color", glm::vec3(1.0f, 0.95f, 0.6f));
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

// a simple flat-screen monitor on the desk, opposite the lamp
void drawMonitor(unsigned int& cubeVAO, Shader& lightingShader, Shader& constantShader)
{
    glm::vec3 monitorBase(-0.45f, legH + topH, tableZ - 0.15f);
    glm::vec3 standColor(0.15f, 0.15f, 0.15f);

    // stand + neck (normal lit dark gray)
    drawCube(cubeVAO, lightingShader, boxModel(monitorBase, glm::vec3(0.12f, 0.03f, 0.12f)), standColor.r, standColor.g, standColor.b);
    glm::vec3 neckPos = monitorBase + glm::vec3(0.0f, 0.03f, 0.0f);
    drawCube(cubeVAO, lightingShader, boxModel(neckPos, glm::vec3(0.03f, 0.15f, 0.03f)), standColor.r, standColor.g, standColor.b);

    // dark bezel behind the screen
    glm::vec3 bezelPos = neckPos + glm::vec3(0.0f, 0.15f, 0.0f);
    drawCube(cubeVAO, lightingShader, boxModel(bezelPos, glm::vec3(0.42f, 0.26f, 0.02f)), 0.08f, 0.08f, 0.08f);

   
    glm::mat4 screenModel = boxModel(bezelPos + glm::vec3(0.0f, 0.015f, -0.015f), glm::vec3(0.36f, 0.20f, 0.01f));
    constantShader.use();
    constantShader.setMat4("model", screenModel);
    constantShader.setVec3("color", glm::vec3(0.65f, 0.85f, 1.0f));
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}


void drawBookshelf(unsigned int& cubeVAO, Shader& shader)
{
    glm::vec3 shelfPos(-1.5f, 0.0f, roomBackZ - 0.2f);
    float width = 1.0f, depth = 0.3f, height = 1.8f;
    float panelThickness = 0.04f;
    glm::vec3 woodColor(0.40f, 0.26f, 0.13f);

    // left and right side panels
    drawCube(cubeVAO, shader, boxModel(shelfPos + glm::vec3(-width / 2.0f + panelThickness / 2.0f, 0.0f, 0.0f), glm::vec3(panelThickness, height, depth)), woodColor.r, woodColor.g, woodColor.b);
    drawCube(cubeVAO, shader, boxModel(shelfPos + glm::vec3(width / 2.0f - panelThickness / 2.0f, 0.0f, 0.0f), glm::vec3(panelThickness, height, depth)), woodColor.r, woodColor.g, woodColor.b);

    // horizontal shelf boards (bottom, top, and evenly spaced in between)
    const int numShelves = 4;
    float shelfSpacing = height / (numShelves - 1);
    for (int i = 0; i < numShelves; i++)
    {
        float shelfY = shelfSpacing * i;
        drawCube(cubeVAO, shader, boxModel(shelfPos + glm::vec3(0.0f, shelfY, 0.0f), glm::vec3(width, panelThickness, depth)), woodColor.r, woodColor.g, woodColor.b);
    }

 
    float bookColor[7][3] = {
        {0.75f, 0.15f, 0.15f}, // red
        {0.15f, 0.45f, 0.75f}, // blue
        {0.85f, 0.65f, 0.15f}, // yellow
        {0.25f, 0.65f, 0.35f}, // green
        {0.55f, 0.25f, 0.65f}, // purple
        {0.85f, 0.45f, 0.20f}, // orange
        {0.20f, 0.55f, 0.55f}  // teal
    };
    float bookHeights[7] = { 0.28f, 0.24f, 0.30f, 0.22f, 0.27f, 0.25f, 0.29f }; // slight height variation
    const int numBooks = 7;
    float bookW = 0.08f, bookD = depth - 0.05f;

    for (int shelfIndex = 0; shelfIndex < numShelves - 1; shelfIndex++) // one row per compartment
    {
        float shelfTopY = shelfSpacing * shelfIndex + panelThickness / 2.0f;
        float startX = -width / 2.0f + panelThickness + 0.06f;
        for (int b = 0; b < numBooks; b++)
        {
            float bx = startX + b * (bookW + 0.02f);
            float bookH = bookHeights[(b + shelfIndex) % numBooks]; // shift the pattern per shelf so rows don't look identical
            drawCube(cubeVAO, shader, boxModel(shelfPos + glm::vec3(bx, shelfTopY, 0.0f), glm::vec3(bookW, bookH, bookD)),
                bookColor[(b + shelfIndex) % numBooks][0], bookColor[(b + shelfIndex) % numBooks][1], bookColor[(b + shelfIndex) % numBooks][2]);
        }
    }
}



void drawFan(unsigned int& cubeVAO, Shader& shader, float angle)
{
    float rodBaseY = fanCenter.y + 0.10f;
    drawCube(cubeVAO, shader, boxModel(fanCenter + glm::vec3(0.0f, 0.10f, 0.0f), glm::vec3(0.05f, roomHeight - rodBaseY, 0.05f)), 0.25f, 0.25f, 0.25f);

    
    drawCube(cubeVAO, shader, boxModel(fanCenter, glm::vec3(0.18f, 0.10f, 0.18f)), 0.30f, 0.30f, 0.30f);

    float bladeColor[3][3] = {
        {0.80f, 0.20f, 0.20f},  // red
        {0.20f, 0.70f, 0.30f},  // green
        {0.20f, 0.40f, 0.80f}   // blue
    };

    for (int i = 0; i < 3; i++)
    {
        float bladeAngle = angle + i * 120.0f;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), fanCenter + glm::vec3(0.0f, 0.05f, 0.0f));
        model = glm::rotate(model, glm::radians(bladeAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // 3D TRANSFORMATION: rotation (spin)
        model = glm::translate(model, glm::vec3(0.55f, 0.0f, 0.0f));                        // push the blade outward from the hub
        model = glm::scale(model, glm::vec3(0.90f, 0.03f, 0.15f));                          // 3D TRANSFORMATION: scaling (flatten into a blade)
        model = glm::translate(model, glm::vec3(-0.5f, 0.0f, -0.5f));                       // center the blade on its own local origin

        drawCube(cubeVAO, shader, model, bladeColor[i][0], bladeColor[i][1], bladeColor[i][2]);
    }
}


void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // WASD moves the camera -- this is the viewing transformation in action
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // arrow keys turn the camera (look around) -- keyboard replacement for mouse-look
    float turnSpeed = 60.0f * deltaTime; // degrees per second
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.ProcessKeyboardRotate(-turnSpeed, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.ProcessKeyboardRotate(turnSpeed, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.ProcessKeyboardRotate(0.0f, turnSpeed);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.ProcessKeyboardRotate(0.0f, -turnSpeed);
}

// key PRESS events (not held-down polling) -- used for on/off toggles so
// they don't flicker while the key is held.
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        if (lampLight.isLightOn()) lampLight.turnOff();
        else lampLight.turnOn();
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        if (sunLight.isLightOn()) sunLight.turnOff();
        else sunLight.turnOn();
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        fanOn = !fanOn;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
