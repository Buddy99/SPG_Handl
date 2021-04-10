#include "main.h"

int main()
{
    if (initialization() == -1)
    {
        return -1;
    }

    setupShadersTexturesBuffers();

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

    // Camera
    camera.UpdateRotation({ 0.87f, -0.0f, -0.03f, 0.47f });

    return 0;
}

void setupShadersTexturesBuffers()
{
    // Load shaders

    // Shader for generating the 3D Texture on the GPU
    rockShader = new Shader();
    rockShader->load("rock.vs", "rock.fs");

    // Shader for Marching Cubes Algorithm on the GPU
    shader = new Shader();
    shader->load("vertexshader.vs", "fragmentshader.fs", "geometryshader.gs");

    // Shader for displacement mapping
    displacementShader = new Shader();
    displacementShader->load("displacement.vs", "displacement.fs");

    // Load textures
    diffuseMap = loadTexture(std::filesystem::absolute("resources/stone_color.jpg").string().c_str());
    normalMap = loadTexture(std::filesystem::absolute("resources/stone_normal.jpg").string().c_str());
    heightMap = loadTexture(std::filesystem::absolute("resources/stone_displacement.jpg").string().c_str());

    // 3D texture
    glGenTextures(1, &tex3D);
    glBindTexture(GL_TEXTURE_3D, tex3D);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, TEX_WIDTH, TEX_HEIGHT, TEX_DEPTH, 0, GL_RED, GL_FLOAT, nullptr);

    // Framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    GLenum c = GL_COLOR_ATTACHMENT0;
    glFramebufferTexture3D(GL_FRAMEBUFFER, c, GL_TEXTURE_3D, tex3D, 0, 0);
    glDrawBuffers(1, &c);

    // VAO & VBO
    glGenVertexArrays(1, &VAORock);
    glGenBuffers(1, &VBORock);
    // Bind Vertex Array Object
    glBindVertexArray(VAORock);
    // Copy the vertices array in a vertex buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBORock);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesRock), verticesRock, GL_STATIC_DRAW);
    // Set the vertex attributes pointers
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
        glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT); 

        // Use the rock Shader to generate the 3D texture on the GPU
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClear(GL_COLOR_BUFFER_BIT);
        rockShader->use();
        rockShader->setFloat("height", height);

        for (int i = 0; i < TEX_DEPTH; i++)
        {
            // Set layer (depending on texture depth)
            rockShader->setFloat("layer", float(i) / float(TEX_DEPTH - 1.0f));

            // Attach the texture to currently bound framebuffer object
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, tex3D, 0, i);

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
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        shader->use();
        shader->setMat4("proj", projection);
        shader->setMat4("view", view);
        shader->setMat4("model", model);
        shader->setInt("texWidth", TEX_WIDTH);
        shader->setInt("texHeight", TEX_HEIGHT);
        shader->setInt("texDepth", TEX_DEPTH);

        // Draw in Wireframe-Mode if Space is pressed
        if (wireframeMode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, tex3D);
        //glPointSize(5.0f);
        glBindVertexArray(VAORock);      
        glDrawArrays(GL_POINTS, 0, TEX_WIDTH * TEX_HEIGHT * TEX_DEPTH);   // draw points 
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // Displacement Mapping
        // --------------------

        // Configure view/projection matrices
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        displacementShader->use();
        displacementShader->setMat4("projection", projection);
        displacementShader->setMat4("view", view);

        // Render displacement-mapped quad
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        displacementShader->setMat4("model", model);
        displacementShader->setVec3("viewPos", camera.Position);
        displacementShader->setVec3("lightPos", lightPos);
        displacementShader->setFloat("heightScale", heightScale);           // Adjust with M and N keys
        displacementShader->setFloat("steps", steps);                       // Adjust with K and J keys
        displacementShader->setFloat("refinementSteps", refinementSteps);   // Adjust with O and I keys

        //std::cout << heightScale << std::endl;
        //std::cout << steps << std::endl;
        //std::cout << refinementSteps << std::endl;

        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, heightMap);

        renderQuad();

        // Glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        keyHandler.FrameUpdate();
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Print error if an error occurs
        printError();
    }
}

void onExit()
{
    // De-allocation of resources
    glDeleteVertexArrays(1, &VAORock);
    glDeleteBuffers(1, &VBORock);

    // Delete shaders
    delete shader;
    delete rockShader;
    delete displacementShader;
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

    // Movement
    if (keyHandler.IsKeyDown(GLFW_KEY_W))
    {
        camera.ProcessKeyboard(Camera::Camera_Movement::FORWARD, deltaTime);
        
    }
    if (keyHandler.IsKeyDown(GLFW_KEY_S))
    {
        camera.ProcessKeyboard(Camera::Camera_Movement::BACKWARD, deltaTime);
    }
    if (keyHandler.IsKeyDown(GLFW_KEY_A))
    {
        camera.ProcessKeyboard(Camera::Camera_Movement::LEFT, deltaTime);
    }
    if (keyHandler.IsKeyDown(GLFW_KEY_D))
    {
        camera.ProcessKeyboard(Camera::Camera_Movement::RIGHT, deltaTime);
    }
    if (keyHandler.IsKeyDown(GLFW_KEY_Q))
    {
        camera.ProcessKeyboard(Camera::Camera_Movement::UP, deltaTime);
    }
    if (keyHandler.IsKeyDown(GLFW_KEY_E))
    {
        camera.ProcessKeyboard(Camera::Camera_Movement::DOWN, deltaTime);
    }

    // Generate more of the 3D texture
    if (keyHandler.IsKeyDown(GLFW_KEY_UP))
    {
        height -= deltaTime;
    }
    if (keyHandler.IsKeyDown(GLFW_KEY_DOWN))
    {
        height += deltaTime;
    }

    // Decrease the displacement height
    if (keyHandler.IsKeyDown(GLFW_KEY_N))
    {
        if (heightScale > 0.0f)
        {
            heightScale -= 0.005f;
        }           
        else
        {
            heightScale = 0.0f;
        }    
    }
    // Increase the displacement height
    if (keyHandler.IsKeyDown(GLFW_KEY_M))
    {
        if (heightScale < 1.0f)
        {
            heightScale += 0.005f;
        }   
        else
        {
            heightScale = 1.0f;
        }         
    }

    // Decrease the displacement mapping steps
    if (keyHandler.WasKeyReleased(GLFW_KEY_J))
    {
        if (steps > 1.0f)
        {
            steps -= 1.0f;
        }
        else
        {
            steps = 1.0f;
        }
    }
    // Increase the displacement mapping steps
    if (keyHandler.WasKeyReleased(GLFW_KEY_K))
    {
        if (steps < 50.0f)
        {
            steps += 1.0f;
        }
        else
        {
            steps = 50.0f;
        }
    }

    // Decrease the displacement mapping refinement steps
    if (keyHandler.WasKeyReleased(GLFW_KEY_I))
    {
        if (refinementSteps > 1.0f)
        {
            refinementSteps -= 1.0f;
        }
        else
        {
            refinementSteps = 1.0f;
        }
    }
    // Increase the displacement mapping refinement steps
    if (keyHandler.WasKeyReleased(GLFW_KEY_O))
    {
        if (refinementSteps < 50.0f)
        {
            refinementSteps += 1.0f;
        }
        else
        {
            refinementSteps = 50.0f;
        }
    }

    // Get Position and rotation of the camera
    if (keyHandler.WasKeyReleased(GLFW_KEY_C))
    {
        std::cout << camera.Position.x << "f, " << camera.Position.y << "f, " << camera.Position.z << "f" << std::endl;
        std::cout << camera.Rotation.w << "f, " << camera.Rotation.x << "f, " << camera.Rotation.y << "f, " << camera.Rotation.z << "f" << std::endl;
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

// Utility function for loading a 2D texture from a file
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// Renders a 1x1 quad with manually calculated tangent vectors
void renderQuad()
{
    if (quadVAO == 0)
    {
        // Positions
        glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
        glm::vec3 pos3(1.0f, -1.0f, 0.0f);
        glm::vec3 pos4(1.0f, 1.0f, 0.0f);
        // Texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // Normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // Calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;

        // Triangle 1
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);

        // Triangle 2
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = glm::normalize(tangent2);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);


        float quadVertices[] = {
            // Positions            // Normal         // Texcoords  // Tangent                          // Bitangent
            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };

        // Configure plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void printError()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << err;
    }
}