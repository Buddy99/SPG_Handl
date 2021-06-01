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
    glDisable(GL_CULL_FACE);

    // Camera
    //camera.UpdateRotation({ 0.87f, -0.0f, -0.03f, 0.47f });
    camera.Position = glm::vec3(-5.0f, 15.0f, -30.0f);
    camera.Yaw = 90.0f;
    camera.Pitch = -18.0f;
    camera.updateCameraVectors();
    lightPos = glm::vec3(0.0f, 30.0f, -30.0f);
    lightDir = glm::vec3(0.0f, 0.0f, 0.0f) - lightPos;

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

    // Shader for shadows
    floorShader = new Shader();
    floorShader->load("shader/shadow.vs", "shader/shadow.fs");

    // Shader for blurring
    filterShader = new Shader();
    filterShader->load("shader/blur.vs", "shader/blur.fs");

    // Shader for Tessellation
    tessellationShader = new Shader();
    tessellationShader->load("shader/tessellation.vs", "shader/tessellation.fs", nullptr, "shader/tessellation.tcs", "shader/tessellation.tes");

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
    floorTexture = loadTexture(std::filesystem::absolute("resources/grass.jpg").string().c_str());

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

    // Shadow Map
    depthMap = new Texture2D(0, GL_TEXTURE1);
    glGenTextures(1, &depthMap->mTex);
    depthMap->mShared = true;
    glBindTexture(GL_TEXTURE_2D, depthMap->mTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    tempDepthMap = new Texture2D(0, GL_TEXTURE0);
    glGenTextures(1, &tempDepthMap->mTex);
    tempDepthMap->mShared = true;
    glBindTexture(GL_TEXTURE_2D, tempDepthMap->mTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glGenFramebuffers(1, &depthMapFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tempDepthMap->mTex, 0);

    glGenFramebuffers(1, &depthMapFilterFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFilterFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthMap->mTex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Plane for displacement mapping
    wall = new Plane();
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(135.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(3.0f, 3.0f, 3.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.0f, 0.0f, 5.0f));
    wall->mModelMatrix = modelMatrix;

    // Floor Plane
    shadowReceiver = new Plane();
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(25.0f, 25.0f, 1.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, -5.0f));
    shadowReceiver->mModelMatrix = modelMatrix;

    // Objects 
    firstObject = new Plane();
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.5f, 1.5f, 1.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.0f, 7.0f, 2.0f));
    firstObject->mModelMatrix = modelMatrix;

    secondObject = new Plane();
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 5.0f, 0.0f));
    secondObject->mModelMatrix = modelMatrix;

    thirdObject = new Plane();
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(6.0f, 9.0f, -2.0f));
    thirdObject->mModelMatrix = modelMatrix;

    filterPlane = new Plane();

    // Tessellation Plane
    tessellationPlane = new Plane();
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(3.0f, 3.0f, 3.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(3.0f, 3.0f, 0.0f));
    tessellationPlane->mModelMatrix = modelMatrix;

    // Setup Text Renderer
    textRenderer.LoadShader("shader/text.vs", "shader/text.fs");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    textRenderer.SetProjection(projection, "projection");
    textRenderer.SetupFreetype(std::filesystem::absolute("resources/Consolas.ttf").string());
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

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Handle input
        processInput(window);

        // Update
        updateScene();

        // Render scene from lights view (for shadow mapping)
        shadowPass = 1;
        CAMERA_WIDTH = SHADOW_WIDTH;
        CAMERA_HEIGHT = SHADOW_HEIGHT;
        glViewport(0, 0, CAMERA_WIDTH, CAMERA_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
        glClear(GL_DEPTH_BUFFER_BIT);
        renderScene();

        // Filter the ShadowMap to blur the shadow edges
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFilterFbo);
        glClear(GL_COLOR_BUFFER_BIT);
        filterShader->use();
        filterShader->setFloat("blurAmount", 4);
        tempDepthMap->use();
        filterPlane->Draw();

        // Render scene from cameras view
        shadowPass = 0;
        CAMERA_WIDTH = SCR_WIDTH;
        CAMERA_HEIGHT = SCR_HEIGHT;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, CAMERA_WIDTH, CAMERA_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderScene();

        // Show FPS
        displayFPS();

        // Glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        keyHandler.FrameUpdate();
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Print error if an error occurs
        printError();
    }
}

void updateScene()
{
    // Update 3D Texture

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

    // Update particles
    particleSystem.UpdateParticles(deltaTime);
}

void renderScene()
{
    // Matrices
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
    glm::mat4 view = camera.GetViewMatrix();

    // Light 
    glm::mat4 lightProjection;
    glm::mat4 lightView;
    glm::mat4 lightSpaceMatrix;
    lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, LIGHT_NEAR_PLANE, LIGHT_FAR_PLANE);
    lightView = glm::lookAt(lightPos, lightDir + lightPos, glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;

    // 3D Texture
    // --------------------

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Generate Triangles using Marching Cubes Algorithm on GPU
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

    // Tessellation
    // ------------------

    // Render plane
    tessellationShader->use();
    tessellationShader->setMat4("projection", projection);
    tessellationShader->setMat4("view", view);
    tessellationShader->setFloat("tessellationFactor", tessellationFactor);
    tessellationShader->setMat4("model", tessellationPlane->mModelMatrix);

    // Draw in Wireframe-Mode if Space is pressed
    if (wireframeMode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    tessellationPlane->Draw(true);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Shadows
    // ------------------

    // Render floor
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    depthMap->use();
    floorShader->use();
    floorShader->setVec4("uColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    floorShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    floorShader->setVec3("lightPos", lightPos);
    floorShader->setVec3("viewPos", camera.Position);
    floorShader->setFloat("shadowPass", shadowPass);
    floorShader->setMat4("projection", projection);
    floorShader->setMat4("view", view);
    floorShader->setMat4("model", shadowReceiver->mModelMatrix);
    shadowReceiver->Draw();

    // Render objects
    floorShader->setVec4("uColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    floorShader->setMat4("model", firstObject->mModelMatrix);
    firstObject->Draw();

    floorShader->setVec4("uColor", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    floorShader->setMat4("model", secondObject->mModelMatrix);
    secondObject->Draw();

    floorShader->setVec4("uColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    floorShader->setMat4("model", thirdObject->mModelMatrix);
    thirdObject->Draw();

    // Particle System
    // --------------------

    // Render particles
    particleSystem.SetMatrices(projection, view, camera.Front, camera.Up);
    particleSystem.RenderParticles();
}

void onExit()
{
    // De-allocation of resources
    glDeleteVertexArrays(1, &VAORock);
    glDeleteBuffers(1, &VBORock);
    glDeleteFramebuffers(1, &depthMapFbo);
    glDeleteFramebuffers(1, &depthMapFilterFbo);

    // Delete shaders
    delete shader;
    delete rockShader;
    delete displacementShader;
    delete floorShader;
    delete filterShader;
    delete tessellationShader;

    // Delete planes
    delete wall;
    delete shadowReceiver;
    delete firstObject;
    delete secondObject;
    delete thirdObject;
    delete filterPlane;
    delete tessellationPlane;

    // Delete textures
    delete depthMap;
    delete tempDepthMap;
}

void displayFPS()
{
    glEnable(GL_BLEND);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    std::string text;

    if (frameTime >= 0.5)
    {
        fps = static_cast<int>(frameCount / frameTime);
        frameTime = 0;
        frameCount = 0;
    }
    frameTime += deltaTime;
    frameCount++;

    text = "FPS:" + std::to_string(fps);
    textRenderer.RenderText(text, SCR_WIDTH - 80.0f, SCR_HEIGHT - 20.0f, 0.4f, glm::vec3(1.0f, 1.0f, 1.0f));

    text = "+";
    textRenderer.RenderText(text, SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

    glDisable(GL_BLEND);
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
    if (keyHandler.IsKeyDown(GLFW_KEY_J))
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
    if (keyHandler.IsKeyDown(GLFW_KEY_K))
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
    if (keyHandler.IsKeyDown(GLFW_KEY_I))
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
    if (keyHandler.IsKeyDown(GLFW_KEY_O))
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
    if (keyHandler.IsKeyDown(GLFW_KEY_G))
    {
        if (particleSystem.mUpdateRate > 0.01f)
        {
            particleSystem.mUpdateRate -= 0.01f;
        }
        else
        {
            particleSystem.mUpdateRate = 0.01f;
        }
    }
    // Increase time it takes to spawn a new generation of particles (update rate)
    if (keyHandler.IsKeyDown(GLFW_KEY_H))
    {
        if (particleSystem.mUpdateRate < 1.0f)
        {
            particleSystem.mUpdateRate += 0.01f;
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
        if (shadowReceiver->Intersects(ray, ray.mHitDistance))
        {
            glm::vec3 hitPos = ray.mOrigin + ray.mDirection * ray.mHitDistance;
            ray.UpdateLine();
            particleSystem.SetGeneratorPosition(hitPos);
        }
    }

    // Tessellation
    // ------------

    // Decrease the tessellation factor
    if (keyHandler.IsKeyDown(GLFW_KEY_V))
    {
        if (tessellationFactor > 0.0f)
        {
            tessellationFactor -= 0.1f;
        }
        else
        {
            tessellationFactor = 0.0f;
        }
    }
    // Increase the tessellation factor
    if (keyHandler.IsKeyDown(GLFW_KEY_B))
    {
        if (tessellationFactor < 50.0f)
        {
            tessellationFactor += 0.1f;
        }
        else
        {
            tessellationFactor = 50.0f;
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