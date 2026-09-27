// ==========================================================================
// Mini Project: Study Corner
// A small room (floor, side walls, back wall with a window, ceiling) with a
// desk (lamp + monitor), a bookshelf, and a continuously spinning ceiling
// fan (the animated object). Lit by a POINT LIGHT (the desk lamp) and a
// DIRECTIONAL LIGHT (sunlight through the window).
//
// The window opening is left empty (no solid "glass" cube) so a small
// outdoor scene -- an overcast sky, a distant tree line, and a handful of
// falling rain streaks -- shows through it. That outdoor scene is built the
// exact same way as everything else here: plain cubes moved around with
// translate/rotate/scale, nothing new.
//
// Built the same way as the "Lighting" project taught in class:
//   - same cube geometry (position + normal)
//   - same Shader / Camera classes
//   - same style of light helper classes (PointLight, DirectionalLight)
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

// ---------------------------------------------------------------------
// function prototypes
// ---------------------------------------------------------------------
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

// ---------------------------------------------------------------------
// settings
// ---------------------------------------------------------------------
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ---------------------------------------------------------------------
// camera (this IS our viewing transformation) -- keyboard-controlled only,
// same idea as the WASD + arrow-key controls used for the cube/camera
// exercises in class, just no mouse involved.
// ---------------------------------------------------------------------
Camera camera(glm::vec3(0.0f, 1.6f, -5.0f));

// ---------------------------------------------------------------------
// timing
// ---------------------------------------------------------------------
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ---------------------------------------------------------------------
// fixed scene coordinates (defined once so the lamp geometry and the
// point light position always agree, and so the fan geometry and the
// fan's rotation always agree)
// ---------------------------------------------------------------------
const float roomHalfWidth = 5.0f;    // side walls at x = -5 and x = +5
const float roomBackZ = 4.5f;    // back wall (the one with the window)
const float roomFrontZ = -5.0f;   // where the floor/walls start (open side, camera enters here)
const float roomCenterZ = (roomFrontZ + roomBackZ) / 2.0f; // -0.25, used to size floor/ceiling/side walls
const float roomDepth = roomBackZ - roomFrontZ;       // 9.5
const float roomHeight = 4.0f;    // wall height = ceiling height

// the window opening in the back wall (shared by drawBackWallWithWindow and
// drawWindowScenery, and by the render loop's rain-loop wrap-around, so all
// three always agree on where the opening is)
const float windowWidth = 1.6f;
const float windowHeight = 1.4f;
const float windowBottomY = 1.3f;
const float windowTopY = windowBottomY + windowHeight;

const float tableZ = 1.5f;      // how far from the camera the table sits
const float legH = 0.70f;      // table leg height
const float topH = 0.06f;      // table top thickness
const float topD = 0.8f;       // table top depth (Z size) -- shared with drawChair so the
                                // chair always lines up with the table's front edge

const glm::vec3 lampBasePos(0.40f, legH + topH, tableZ + 0.20f);
const glm::vec3 lampStandPos = lampBasePos + glm::vec3(0.0f, 0.03f, 0.0f);
const glm::vec3 lampBulbPos = lampStandPos + glm::vec3(0.0f, 0.28f, 0.0f);
const glm::vec3 pointLightPos = lampBulbPos + glm::vec3(0.0f, 0.05f, 0.0f); // ~ center of bulb

const glm::vec3 fanCenter(0.0f, 2.3f, tableZ);

// ---------------------------------------------------------------------
// animation state
// ---------------------------------------------------------------------
float fanRotationAngle = 0.0f;
float fanSpeed = 90.0f; // degrees per second
bool fanOn = true;

float rainOffset = 0.0f; // how far the rain streaks have fallen (wraps every windowHeight units)
float rainSpeed = 1.0f;  // window-heights per second

// ---------------------------------------------------------------------
// the two required lights
// ---------------------------------------------------------------------
PointLight lampLight(
    pointLightPos.x, pointLightPos.y, pointLightPos.z,   // position
    0.05f, 0.05f, 0.03f,     // ambient  (dim warm)
    0.90f, 0.75f, 0.40f,     // diffuse  (warm yellow-orange, like a bulb)
    1.00f, 0.90f, 0.60f,     // specular
    1.0f,     // k_c
    0.09f,    // k_l
    0.032f    // k_q
);

DirectionalLight sunLight(
    -0.3f, -1.0f, 0.4f,      // direction the light travels (down + sideways)
    0.15f, 0.15f, 0.18f,     // ambient  (soft cool tint)
    0.55f, 0.55f, 0.50f,     // diffuse  (pale sunlight)
    0.30f, 0.30f, 0.30f      // specular
);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
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
    // no mouse callbacks are registered -- this project is keyboard-only,
    // so the cursor is left as a normal, free-moving system cursor.

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // build and compile our shader programs
    // ---------------------------------------
    Shader lightingShader("vertexShaderForPhongShading.vs", "fragmentShaderForPhongShading.fs");
    Shader constantShader("vertexShader.vs", "fragmentShader.fs");

    // ---------------------------------------------------------------------
    // set up one unit cube (position + normal per vertex).
    // every object in the scene (floor, wall, table, lamp, fan) is just
    // this SAME cube, transformed differently with translate/rotate/scale.
    // ---------------------------------------------------------------------
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

    // ---------------------------------------------------------------------
    // render loop
    // ---------------------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // advance the fan's rotation angle EVERY frame -> continuous animation
        if (fanOn)
        {
            fanRotationAngle += fanSpeed * deltaTime;
            if (fanRotationAngle > 360.0f)
                fanRotationAngle -= 360.0f;
        }

        // advance the rain EVERY frame -> continuous animation (same idea as
        // the fan: a number that keeps growing, wrapped back into range)
        rainOffset += rainSpeed * deltaTime;
        if (rainOffset > windowHeight)
            rainOffset -= windowHeight;

        // render
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ----- viewing transformation: build projection + view from the camera -----
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // ----- lit objects (floor, wall, table, lamp body, fan) -----
        lightingShader.use();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setVec3("viewPos", camera.Position);

        lampLight.setUpPointLight(lightingShader);
        sunLight.setUpDirectionalLight(lightingShader);

        // the constant-color shader (used only for glowing objects: the bulb
        // and the monitor screen) needs the same projection/view every frame too
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

// ===========================================================================
// small helper: builds a model matrix for a box of size "size" whose BASE
// sits at "pos" and which is centered on X/Z. This is the ONE place that
// combines translate + scale, so every piece of furniture below is a
// one-line call instead of five lines of matrix math.
// ===========================================================================
glm::mat4 boxModel(glm::vec3 pos, glm::vec3 size)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);              // 3D TRANSFORMATION: translation
    model = glm::scale(model, size);                  // 3D TRANSFORMATION: scaling
    model = glm::translate(model, glm::vec3(-0.5f, 0.0f, -0.5f)); // re-center on X/Z
    return model;
}

// draws the shared cube with a given flat color, lit by both lights
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

// ---------------------------------------------------------------------
// scene pieces
// ---------------------------------------------------------------------
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

// the back wall (the one the desk faces) with a rectangular window cut into
// it, directly in front of the desk. Built the same way you'd draw any
// other box -- just several boxes (bands) arranged around a gap, instead of
// one solid box, plus a thin wooden frame.
//
// NOTE: there is no "glass" cube filling the opening any more. A flat,
// opaque colored panel there just looked like a painted square, not glass.
// Instead the opening is left empty, and drawWindowScenery() puts a small
// outdoor scene a little further back (bigger Z) so it shows straight
// through the hole -- the same way you'd see outside through a real window.
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

// what you see THROUGH the window: an overcast sky panel, a low dark strip
// standing in for a distant tree line, and a handful of thin, tilted cube
// "rain streaks" that continuously fall and loop -- built the exact same
// way as the fan blades (translate, then rotate, then scale, then re-center),
// just moving in a straight line down instead of spinning around a hub.
void drawWindowScenery(unsigned int& cubeVAO, Shader& shader, float rainOffset)
{
    float skyZ = roomBackZ + 0.6f;   // outside the room, behind the window opening
    float rainZ = roomBackZ + 0.25f; // between the opening and the sky panel

    // overcast rainy sky -- a little bigger than the opening so it fills the
    // view through the window from normal standing/viewing positions
    drawCube(cubeVAO, shader,
        boxModel(glm::vec3(0.0f, windowBottomY - 0.2f, skyZ), glm::vec3(windowWidth + 0.4f, windowHeight + 0.4f, 0.05f)),
        0.55f, 0.58f, 0.62f);

    // a low, dark strip near the bottom stands in for a distant tree line
    drawCube(cubeVAO, shader,
        boxModel(glm::vec3(0.0f, windowBottomY - 0.2f, skyZ - 0.03f), glm::vec3(windowWidth + 0.4f, 0.35f, 0.05f)),
        0.20f, 0.28f, 0.20f);

    // a handful of rain streaks, spread across the window and staggered so
    // they don't all fall in sync; each one loops from the top of the
    // window back to the top once it reaches the bottom
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
    float topW = 1.2f; // topD (depth) is now a shared global, see the constants near tableZ
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

// a simple chair pulled up to the table, on the side facing the room's
// entrance (smaller Z than the table), so the desk sits between the chair
// and the window/bookshelf on the back wall -- built the exact same way as
// the table: a handful of boxes (4 legs, a seat, a backrest), just like the
// lecture's box-by-box modeling approach.
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

    // backrest -- on the far side from the table, since that's a person's
    // back when they're sitting facing the desk
    float backH = 0.5f, backThick = 0.05f;
    glm::vec3 backPos(chairX, seatH + seatThick, chairZ - seatD / 2.0f + backThick / 2.0f);
    drawCube(cubeVAO, shader, boxModel(backPos, glm::vec3(seatW, backH, backThick)), seatColor.r, seatColor.g, seatColor.b);
}

void drawLamp(unsigned int& cubeVAO, Shader& lightingShader, Shader& constantShader)
{
    // base + stand are lit normally, like any other object
    drawCube(cubeVAO, lightingShader, boxModel(lampBasePos, glm::vec3(0.14f, 0.03f, 0.14f)), 0.20f, 0.20f, 0.20f);
    drawCube(cubeVAO, lightingShader, boxModel(lampStandPos, glm::vec3(0.03f, 0.28f, 0.03f)), 0.20f, 0.20f, 0.20f);

    // the bulb is drawn with the CONSTANT-COLOR shader (not affected by
    // lighting) so it reads as a glowing light source, not a shaded object --
    // the same trick used in the "Lighting" project to draw the lamp cubes.
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

    // glowing screen face -- drawn with the constant shader, same "it's on"
    // trick used for the lamp bulb, sitting flush on the bezel's near side
    glm::mat4 screenModel = boxModel(bezelPos + glm::vec3(0.0f, 0.015f, -0.015f), glm::vec3(0.36f, 0.20f, 0.01f));
    constantShader.use();
    constantShader.setMat4("model", screenModel);
    constantShader.setVec3("color", glm::vec3(0.65f, 0.85f, 1.0f));
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

// a small bookshelf against the BACK wall -- the same wall the window is
// in -- placed just to the left of the window opening, so it sits directly
// ahead of the desk (which faces this same wall) instead of tucked away
// near the room's entrance. 2 side panels, 4 shelf boards, and a full row of
// different-colored, different-height "books" on all 3 shelf compartments
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

    // a row of different-colored books on EVERY shelf compartment (all 3
    // gaps between the 4 shelf boards), each book a slightly different
    // color and height so the shelf reads as "full" -- still just the same
    // shared cube, transformed differently, nothing new introduced.
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


// the ceiling fan: hub + 3 blades. "angle" is fanRotationAngle from the
// render loop, so this whole assembly spins continuously.
void drawFan(unsigned int& cubeVAO, Shader& shader, float angle)
{
    // mount rod, now reaching all the way up to the ceiling (roomHeight)
    float rodBaseY = fanCenter.y + 0.10f;
    drawCube(cubeVAO, shader, boxModel(fanCenter + glm::vec3(0.0f, 0.10f, 0.0f), glm::vec3(0.05f, roomHeight - rodBaseY, 0.05f)), 0.25f, 0.25f, 0.25f);

    // hub
    drawCube(cubeVAO, shader, boxModel(fanCenter, glm::vec3(0.18f, 0.10f, 0.18f)), 0.30f, 0.30f, 0.30f);

    // 3 blades, spaced 120 degrees apart, DIFFERENT COLORS, continuously spinning
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

// ---------------------------------------------------------------------
// input handling
// ---------------------------------------------------------------------
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
