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
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Camera
    // camera.UpdateRotation({ 0.87f, -0.0f, -0.03f, 0.47f });
    camera.Yaw = 90.0f;
    camera.Pitch = -1.5f;
    camera.updateCameraVectors();

    return 0;
}

void setupShadersTexturesBuffers()
{
    // Load shaders

    // Shader for generating the 3D Texture on the GPU
    rockShader = new Shader();
    rockShader->load("shader/rock.vs", "shader/rock.fs");

    // Shader for Marching Cubes Algorithm on the GPU
    shader = new Shader();
    shader->load("shader/vertexshader.vs", "shader/fragmentshader.fs", "shader/geometryshader.gs");

    // Shader for displacement mapping
    displacementShader = new Shader();
    displacementShader->load("shader/displacement.vs", "shader/displacement.fs");

    // Shader for background plane
    backgroundShader = new Shader();
    backgroundShader->load("shader/background.vs", "shader/background.fs");
    backgroundTexture = loadTexture(std::filesystem::absolute("resources/background.jpg").string().c_str());

    // Shader for particle system
    particleSystem.InitParticleSystem();
    // Particle texture
    particleSystem.mTexture = new Texture2D(loadTexture(std::filesystem::absolute("resources/particle.png").string().c_str()), GL_TEXTURE0);

    // Set generator particle attributes
    particleSystem.SetGeneratorProperties(
        glm::vec3(-5.0f, 0.0f, 5.0f),   // Where the particles are generated
        glm::vec3(-2, 0, -2),           // Minimum velocity
        glm::vec3(2, 4, 2),             // Maximum velocity
        glm::vec3(0, -1, 0),            // Gravity force applied to particles
        glm::vec3(1.0f, 1.0f, 1.0f),    // Color
        1.5f,                           // Minimum lifetime in seconds
        2.0f,                           // Maximum lifetime in seconds
        1.0f,                           // Rendered size
        0.3f,                           // Set the update rate to 0.3
        30);                            // And spawn 30 particles

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

    // Plane for displacement mapping
    wall = new Plane();
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(135.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(3.0f, 3.0f, 3.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.0f, 0.0f, 5.0f));
    wall->mModelMatrix = modelMatrix;

    // Background Plane
    background = new Plane();
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(96.0f, -54.0f, 1.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.5f, 1.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 50.0f));
    background->mModelMatrix = modelMatrix;
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

        // 3D Texture
        // --------------------

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

        wall->Draw();

        // Particle System
        // --------------------

        // Render background
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        backgroundShader->use();
        backgroundShader->setMat4("projection", projection);
        backgroundShader->setMat4("view", view);
        backgroundShader->setMat4("model", background->mModelMatrix);
        background->Draw();

        // Update & Render particles
        particleSystem.SetMatrices(projection, view, camera.Front, camera.Up);
        particleSystem.UpdateParticles(deltaTime);
        particleSystem.RenderParticles();

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
    delete backgroundShader;

    // Delete planes
    delete background;
    delete wall;
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

    // 3D-Texture
    // ----------

    // Generate more of the 3D texture
    if (keyHandler.IsKeyDown(GLFW_KEY_UP))
    {
        height -= deltaTime;
    }
    if (keyHandler.IsKeyDown(GLFW_KEY_DOWN))
    {
        height += deltaTime;
    }

    // Toggle Wireframe Mode
    if (keyHandler.WasKeyReleased(GLFW_KEY_SPACE))
    {
        wireframeMode = !wireframeMode;
    }

    // Displacement - Mapping
    // ----------------------

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

    // Particle - System
    // -----------------

    // Decrease time it takes to spawn a new generation of particles (update rate)
    if (keyHandler.WasKeyReleased(GLFW_KEY_G))
    {
        if (particleSystem.mUpdateRate > 0.01f)
        {
            particleSystem.mUpdateRate -= 0.02f;
        }
        else
        {
            particleSystem.mUpdateRate = 0.01f;
        }
    }
    // Increase time it takes to spawn a new generation of particles (update rate)
    if (keyHandler.WasKeyReleased(GLFW_KEY_H))
    {
        if (particleSystem.mUpdateRate < 1.0f)
        {
            particleSystem.mUpdateRate += 0.02f;
        }
        else
        {
            particleSystem.mUpdateRate = 1.0f;
        }
    }

    // Shoot Ray
    if (keyHandler.WasKeyReleased(GLFW_KEY_R))
    {
        ray = Ray(camera.Position, camera.Front);
        ray.UpdateLine();
        // Check if the ray intersects the background
        if (background->Intersects(ray, ray.mHitDistance))
        {
            glm::vec3 hitPos = ray.mOrigin + ray.mDirection * ray.mHitDistance;
            ray.UpdateLine();
            particleSystem.SetGeneratorPosition(hitPos);
        }
    }


    // Get Position and rotation of the camera
    if (keyHandler.WasKeyReleased(GLFW_KEY_C))
    {
        std::cout << camera.Position.x << "f, " << camera.Position.y << "f, " << camera.Position.z << "f" << std::endl;
        std::cout << camera.Rotation.w << "f, " << camera.Rotation.x << "f, " << camera.Rotation.y << "f, " << camera.Rotation.z << "f" << std::endl;
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

void printError()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << err;
    }
}