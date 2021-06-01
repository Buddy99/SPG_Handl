#pragma once
#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <glBasics/shader.h>
#include <glBasics/texture2D.h>
#include <glBasics/camera.h>
#include <glBasics/textRenderer.h>
#include <stb_image.h>

#include <iostream>
#include <filesystem>
#include <vector>

#include "KeyHandler.h"
#include "ParticleSystem.h"
#include "Ray.h"
#include "Plane.h"

int initialization();
void setupShadersTexturesBuffers();
void updateScene();
void renderLoop();
void renderScene();
void displayFPS();
void onExit();

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void handleKey(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadTexture(char const* path);
void printError();

// Settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

const float LIGHT_NEAR_PLANE = 0.5f;
const float LIGHT_FAR_PLANE = 100.0f;

unsigned int CAMERA_WIDTH = 0;
unsigned int CAMERA_HEIGHT = 0;

// Camera
Camera camera;
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// Light
glm::vec3 lightPos;
glm::vec3 lightDir;

// Window
GLFWwindow* window;

// KeyHandler
KeyHandler keyHandler;

// Text Renderer
TextRenderer textRenderer;

// Shader
Shader* rockShader;
Shader* shader;
Shader* displacementShader;
Shader* floorShader;
Shader* filterShader;
Shader* tessellationShader;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float currentFrame;

int fps = 0;
int frameCount = 0;
double frameTime = 0.0f;

// VBOs, VAOs and EBOs
unsigned int VBORock, VAORock;
unsigned int quadVAO = 0;
unsigned int quadVBO;

// framebuffer
unsigned int framebuffer;

// 3D Texture
unsigned int tex3D;
const unsigned int TEX_WIDTH = 96;
const unsigned int TEX_DEPTH = 96;
const unsigned int TEX_HEIGHT = 256;

// Textures
unsigned int diffuseMap;
unsigned int normalMap;
unsigned int heightMap;

float heightScale = 0.1f;

// Steps & Refinement Steps
float steps = 15;
float refinementSteps = 10;

float height = 0;

bool wireframeMode = false;

float verticesRock[6][2] = { {-1.0f, -1.0f}, {-1.0, 1.0}, {1.0, -1.0}, {1.0f, 1.0f}, {-1.0, 1.0}, {1.0, -1.0} };

// Particle System
ParticleSystem particleSystem;

Ray ray;

// Planes
Plane* wall;
Plane* shadowReceiver;
unsigned int floorTexture;
Plane* firstObject;
Plane* secondObject;
Plane* thirdObject;
Plane* filterPlane;

// Tessellation
Plane* tessellationPlane;
float tessellationFactor = 1.0f;

// Shadows
unsigned int depthMapFbo;
unsigned int depthMapFilterFbo;
Texture2D* depthMap;
Texture2D* tempDepthMap;

float shadowPass;