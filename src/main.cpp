#include "main.h"

int main()
{
    if (initialization() == -1)
    {
        return -1;
    }

    generate3DTexture();

    renderLoop();

    onExit();
    glfwTerminate();
    return 0;
}

int initialization()
{
    // Glfw: initialization and configuration
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Glfw create window
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Handl_SPG", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Error: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, handleKey);

    // Mouse capturing
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initiaize glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Error: Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure global opengl state
    glEnable(GL_DEPTH_TEST);
}

void generate3DTexture()
{
    // Load shaders

    // Generate 3D Texture using the rock-Shaders
    rockShader = new Shader();
    rockShader->load("rock.vs", "rock.fs");

    // Shader for Marching Cubes Algorithm on the GPU
    shader = new Shader();
    shader->load("vertexshader.vs", "fragmentshader.fs"/*, "geometryshader.gs"*/);

    // 3D texture
    glGenTextures(1, &tex3D);
    glBindTexture(GL_TEXTURE_3D, tex3D);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 64, 64, 64, 0, GL_RED, GL_FLOAT, nullptr);  // 64x64x64... Texture Size

    // Framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    GLenum c = GL_COLOR_ATTACHMENT0;
    glFramebufferTexture3D(GL_FRAMEBUFFER, c, GL_TEXTURE_3D, tex3D, 0, 0);
    glDrawBuffers(1, &c);

    // VAO & VBO
    glGenVertexArrays(1, &VAORock);
    glGenBuffers(1, &VBORock);
    // bind Vertex Array Object
    glBindVertexArray(VAORock);
    // copy the vertices array in a vertex buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBORock);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesRock), verticesRock, GL_STATIC_DRAW);
    // set the vertex attributes pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
    glBindVertexArray(0);
}

void renderLoop()
{
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate deltaTime
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Handle input
        processInput(window);

        // Set Viewport according to 3D Texture-Size
        glViewport(0, 0, 64, 64); 

        // use the rock Shader to generate the 3D texture on the GPU
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClear(GL_COLOR_BUFFER_BIT);
        rockShader->use();
        rockShader->setFloat("height", height);

        for (int i = 0; i < 64; i++)    // 64... Texture height
        {
            // set layer (depending on texture height)
            rockShader->setFloat("layer", i * (1.0f / 63.0f));

            // attach the texture to currently bound framebuffer object
            glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, tex3D, 0, i);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
            }

            glBindVertexArray(VAORock);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Set viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Generate Triangles using Marching Cubes Algorithm on GPU
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);

        // Draw in Wireframe-Mode if Space is pressed
        if (wireframeMode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, tex3D);
        //glPointSize(5.0f);
        glBindVertexArray(VAORock);      
        glDrawArrays(GL_POINTS, 0, 64*64*64);   // draw points 
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // Glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        keyHandler.FrameUpdate();
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Print error if an error occurs
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cerr << err << std::endl;
        }
    }
}


void onExit()
{
    // De-allocation of resources
    glDeleteVertexArrays(1, &VAORock);
    glDeleteBuffers(1, &VBORock);
}

// Handle Key input
void handleKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (action)
    {
    case GLFW_PRESS:
        keyHandler.KeyPressed(key);
        break;

    case GLFW_RELEASE:
        keyHandler.KeyReleased(key);
        break;

    default:
        break;
    }
}

// Process all input (pressed keys, etc.)
void processInput(GLFWwindow* window)
{
    // Exit
    if (keyHandler.WasKeyReleased(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(window, true);
    }

    // Generate more of the 3D texture
    if (keyHandler.IsKeyDown(GLFW_KEY_W))
    {
        //camera.ProcessKeyboard(Camera::Camera_Movement::UP, deltaTime);
        height -= deltaTime;
    }
    if (keyHandler.IsKeyDown(GLFW_KEY_S))
    {
        //camera.ProcessKeyboard(Camera::Camera_Movement::DOWN, deltaTime);
        height += deltaTime;
    }

    // Toggle Wireframe Mode
    if (keyHandler.WasKeyReleased(GLFW_KEY_SPACE))
    {
        wireframeMode = !wireframeMode;
    }
}

// Handle window size changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Handle mouse movement
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Handle mouse scrolling
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}