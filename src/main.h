#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <glBasics/shader.h>
#include <glBasics/camera.h>

#include <iostream>
#include <filesystem>
#include <vector>

#include "KeyHandler.h"

int initialization();
void generate3DTexture();
void renderLoop();
void onExit();

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void handleKey(GLFWwindow* window, int key, int scancode, int action, int mods);

// Settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Camera
Camera camera(glm::vec3(20.0f, 15.0f, -10.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// Light
//glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);

// Window
GLFWwindow* window;

// KeyHandler
KeyHandler keyHandler;

// Shader
Shader* rockShader;
Shader* shader;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float currentFrame;

// VBOs, VAOs and EBOs
unsigned int VBORock, VAORock;

// framebuffer
unsigned int framebuffer;

// 3D Texture
unsigned int tex3D;
const unsigned int TEX_WIDTH = 96;
const unsigned int TEX_DEPTH = 96;
const unsigned int TEX_HEIGHT = 256;

float height = 0;

bool wireframeMode = false;

float verticesRock[6][2] = { {-1.0f, -1.0f}, {-1.0, 1.0}, {1.0, -1.0}, {1.0f, 1.0f}, {-1.0, 1.0}, {1.0, -1.0} };