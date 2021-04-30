#pragma once
#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <glBasics/shader.h>
#include <glBasics/camera.h>
#include <stb_image.h>

#include <iostream>
#include <filesystem>
#include <vector>

#include "KeyHandler.h"
#include "ParticleSystem.h"

int initialization();
void setupShadersTexturesBuffers();
void renderLoop();
void onExit();

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void handleKey(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadTexture(char const* path);
void renderQuad();
void printError();

// Settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Camera
// Camera camera(glm::vec3(20.0f, 18.0f, -10.0f));
Camera camera(glm::vec3(0.0f, 0.25f, -3.5f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// Light
glm::vec3 lightPos(30.0f, 20.0f, -20.0f);

// Window
GLFWwindow* window;

// KeyHandler
KeyHandler keyHandler;

// Shader
Shader* rockShader;
Shader* shader;
Shader* displacementShader;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float currentFrame;

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