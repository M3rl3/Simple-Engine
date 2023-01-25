#include "OpenGL.h"
#include "cMeshInfo.h"
#include "LoadModel.h"

#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cShaderManager/cShaderManager.h"
#include "cVAOManager/cVAOManager.h"
#include "cBasicTextureManager/cBasicTextureManager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

GLFWwindow* window;
GLint mvp_location = 0;
GLuint shaderID = 0;

cVAOManager* VAOMan;
cBasicTextureManager* TextureMan;

cMeshInfo* skybox_sphere_mesh;
cMeshInfo* player_mesh;
cMeshInfo* bulb_mesh;

unsigned int readIndex = 0;
int object_index = 0;

bool wireFrame = false;
bool doOnce = true;
bool enableMouse = false;
bool mouseClick = false;

std::vector <std::string> meshFiles;
std::vector <cMeshInfo*> meshArray;

void ReadFromFile();
void ReadSceneDescription();
void ManageLights();

enum eEditMode
{
    MOVING_CAMERA,
    MOVING_LIGHT,
    MOVING_SELECTED_OBJECT,
    TAKE_CONTROL
};

glm::vec3 cameraEye; //loaded from external file
//glm::vec3 cameraTarget = glm::vec3(-75.0f, 2.0f, 0.0f);

// controlled by mouse
glm::vec3 cameraTarget = glm::vec3(0.f, 0.f, -1.f);
eEditMode theEditMode = MOVING_CAMERA;

glm::vec3 cameraDest = glm::vec3(0.f);
glm::vec3 cameraVelocity = glm::vec3(0.f);

float yaw = 0.f;
float pitch = 0.f;
float fov = 45.f;

// mouse state
bool firstMouse = true;
float lastX = 800.f / 2.f;
float lastY = 600.f / 2.f;

float beginTime = 0.f;
float currentTime = 0.f;
float timeDiff = 0.f;
int frameCount = 0;

static void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        theEditMode = MOVING_CAMERA;
    }
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
    {
        theEditMode = MOVING_SELECTED_OBJECT;
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        theEditMode = TAKE_CONTROL;
        cameraTarget = player_mesh->position;
        cameraEye = player_mesh->position - glm::vec3(20.f, -4.f, 0.f);
    }
    // Wireframe
    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        for (int i = 0; i < meshArray.size(); i++) {
            meshArray[i]->isWireframe = true;
        }
    }
    if (key == GLFW_KEY_X && action == GLFW_RELEASE) {
        for (int i = 0; i < meshArray.size(); i++) {
            meshArray[i]->isWireframe = false;
        }
    }
    if (key == GLFW_KEY_LEFT_ALT) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    /* 
    *    updates translation of all objects in the scene based on changes made to scene 
    *    description files, resets everything if no changes were made
    */
    if (key == GLFW_KEY_U && action == GLFW_PRESS) {
        ReadSceneDescription();
    }
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        enableMouse = !enableMouse;
    }

    switch (theEditMode)
    {
        case MOVING_CAMERA:
        {
            const float CAMERA_MOVE_SPEED = 1.f;
            if (key == GLFW_KEY_A)     // Left
            {
                ::cameraEye.x -= CAMERA_MOVE_SPEED;
            }
            if (key == GLFW_KEY_D)     // Right
            {
                ::cameraEye.x += CAMERA_MOVE_SPEED;
            }
            if (key == GLFW_KEY_W)     // Forward
            {
                ::cameraEye.z += CAMERA_MOVE_SPEED;
            }
            if (key == GLFW_KEY_S)     // Backwards
            {
                ::cameraEye.z -= CAMERA_MOVE_SPEED;
            }
            if (key == GLFW_KEY_Q)     // Down
            {
                ::cameraEye.y -= CAMERA_MOVE_SPEED;
            }
            if (key == GLFW_KEY_E)     // Up
            {
                ::cameraEye.y += CAMERA_MOVE_SPEED;
            }

            if (key == GLFW_KEY_1)
            {
                ::cameraEye = glm::vec3(0.f, 0.f, -5.f);
            }
        }
        break;
        case MOVING_SELECTED_OBJECT:
        {
            const float OBJECT_MOVE_SPEED = 1.f;
            if (key == GLFW_KEY_A)     // Left
            {
                meshArray[object_index]->position.x -= OBJECT_MOVE_SPEED;
            }
            if (key == GLFW_KEY_D)     // Right
            {
                meshArray[object_index]->position.x += OBJECT_MOVE_SPEED;
            }
            if (key == GLFW_KEY_W)     // Forward
            {
                meshArray[object_index]->position.z += OBJECT_MOVE_SPEED;
            }
            if (key == GLFW_KEY_S)     // Backwards
            {
                meshArray[object_index]->position.z -= OBJECT_MOVE_SPEED;
            }
            if (key == GLFW_KEY_Q)     // Down
            {
                meshArray[object_index]->position.y -= OBJECT_MOVE_SPEED;
            }
            if (key == GLFW_KEY_E)     // Up
            {
                meshArray[object_index]->position.y += OBJECT_MOVE_SPEED;
            }

            // Cycle through objects in the scene
            if (key == GLFW_KEY_1 && action == GLFW_PRESS)
            {
                if (!enableMouse) cameraTarget = glm::vec3(0.f, 0.f, 0.f);
            }
            if (key == GLFW_KEY_2 && action == GLFW_PRESS)
            {
                object_index++;
                if (object_index > meshArray.size()-1) {
                    object_index = 0;
                }
                if (!enableMouse) cameraTarget = meshArray[object_index]->position;
            }
            if (key == GLFW_KEY_3 && action == GLFW_PRESS)
            {
                object_index--;
                if (object_index < 0) {
                    object_index = meshArray.size() - 1;
                }
                if (!enableMouse) cameraTarget = meshArray[object_index]->position;
            }    
        }
        break;
        case TAKE_CONTROL: {
            if (key == GLFW_KEY_W) {
                player_mesh->position.x += 1.f;
            }
            if (key == GLFW_KEY_S) {
                player_mesh->position.x -= 1.f;
            }
            if (key == GLFW_KEY_A) {
                player_mesh->position.z -= 1.f;
            }
            if (key == GLFW_KEY_D) {
                player_mesh->position.z += 1.f;
            }
            if (key == GLFW_KEY_Q) {
                player_mesh->position.y += 1.f;
            }
            if (key == GLFW_KEY_E) {
                player_mesh->position.y -= 1.f;
            }
        }
        break;
    }
}

static void MouseCallBack(GLFWwindow* window, double xposition, double yposition) {

    if (firstMouse) {
        lastX = xposition;
        lastY = yposition;
        firstMouse = false;
    }

    float xoffset = xposition - lastX;
    float yoffset = lastY - yposition;  // reversed since y coordinates go from bottom to up
    lastX = xposition;
    lastY = yposition;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // prevent perspective from getting flipped by capping it
    if (pitch > 89.f) {
        pitch = 89.f;
    }
    if (pitch < -89.f) {
        pitch = -89.f;
    }

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    if (enableMouse) {
        cameraTarget = glm::normalize(front);
    }
}

static void ScrollCallBack(GLFWwindow* window, double xoffset, double yoffset) {
    if (fov >= 1.f && fov <= 45.f) {
        fov -= yoffset;
    }
    if (fov <= 1.f) {
        fov = 1.f;
    }
    if (fov >= 45.f) {
        fov = 45.f;
    }
}

void Initialize() {

    if (!glfwInit()) {
        std::cerr << "GLFW init failed." << std::endl;
        glfwTerminate();
        return;
    }

    const char* glsl_version = "#version 420";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* currentMonitor = glfwGetPrimaryMonitor();

    const GLFWvidmode* mode = glfwGetVideoMode(currentMonitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    window = glfwCreateWindow(1366, 768, "Simple-Engine", NULL, NULL);

    // Uncomment for fullscreen support based on current monitor
    // window = glfwCreateWindow(mode->height, mode->width, "Simple-Engine", currentMonitor, NULL);
    
    if (!window) {
        std::cerr << "Window creation failed." << std::endl;
        glfwTerminate();
        return;
    }

    glfwSetWindowAspectRatio(window, 16, 9);

    // keyboard callback
    glfwSetKeyCallback(window, KeyCallback);

    // mouse and scroll callback
    glfwSetCursorPosCallback(window, MouseCallBack);
    glfwSetScrollCallback(window, ScrollCallBack);

    // capture mouse input
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetErrorCallback(ErrorCallback);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress))) {
        std::cerr << "Error: unable to obtain pocess address." << std::endl;
        return;
    }
    glfwSwapInterval(1); //vsync
}

void Render() {
    
    GLint vpos_location = 0;
    GLint vcol_location = 0;
    GLuint vertex_buffer = 0;

    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    //Shader Manager
    cShaderManager* shadyMan = new cShaderManager();

    cShaderManager::cShader vertexShader;
    cShaderManager::cShader fragmentShader;

    vertexShader.fileName = "./shaders/vertexShader.glsl";
    fragmentShader.fileName = "./shaders/fragmentShader.glsl";

    if (!shadyMan->createProgramFromFile("ShadyProgram", vertexShader, fragmentShader)) {
        std::cout << "Error: Shader program failed to compile." << std::endl;
        std::cout << shadyMan->getLastError();
        return;
    }
    else {
        std::cout << "Shaders compiled." << std::endl;
    }

    shadyMan->useShaderProgram("ShadyProgram");
    shaderID = shadyMan->getIDFromFriendlyName("ShadyProgram");
    glUseProgram(shaderID);

    // Load asset paths from external file
    ReadFromFile();

    // VAO Manager
    VAOMan = new cVAOManager();
    
    // Scene
    sModelDrawInfo bulb;
    LoadModel(meshFiles[0], bulb);
    if (!VAOMan->LoadModelIntoVAO("bulb", bulb, shaderID)) {
        std::cerr << "Could not load model into VAO" << std::endl;
    }
    bulb_mesh = new cMeshInfo();
    bulb_mesh->meshName = "bulb";
    bulb_mesh->friendlyName = "bulb";
    meshArray.push_back(bulb_mesh);
    
    sModelDrawInfo terrain_obj;
    LoadModel(meshFiles[3], terrain_obj);
    if (!VAOMan->LoadModelIntoVAO("terrain", terrain_obj, shaderID)) {
        std::cerr << "Could not load model into VAO" << std::endl;
    }
    cMeshInfo* terrain_mesh = new cMeshInfo();
    terrain_mesh->meshName = "terrain";
    terrain_mesh->RGBAColour = glm::vec4(25.f, 25.f, 25.f, 1.f);
    terrain_mesh->doNotLight = false;
    terrain_mesh->useRGBAColour = true;
    meshArray.push_back(terrain_mesh);

    sModelDrawInfo player_obj;
    LoadModel(meshFiles[7], player_obj);
    if (!VAOMan->LoadModelIntoVAO("player", player_obj, shaderID)) {
        std::cerr << "Could not load model into VAO" << std::endl;
    }
    player_mesh = new cMeshInfo();
    player_mesh->meshName = "player";
    player_mesh->friendlyName = "player";
    player_mesh->hasTexture = true;
    player_mesh->RGBAColour = glm::vec4(200.f, 20.f, 200.f, 1.f);
    player_mesh->useRGBAColour = false;
    player_mesh->textures[0] = "man.bmp";
    player_mesh->textureRatios[0] = 1.f;
    meshArray.push_back(player_mesh);

    sModelDrawInfo moon_obj;
    LoadModel(meshFiles[4], moon_obj);
    if (!VAOMan->LoadModelIntoVAO("moon", moon_obj, shaderID)) {
        std::cerr << "Could not load model into VAO" << std::endl;
    }
    cMeshInfo* moon_mesh = new cMeshInfo();
    moon_mesh->meshName = "moon";
    moon_mesh->friendlyName = "moon";
    moon_mesh->useRGBAColour = false;
    moon_mesh->RGBAColour = glm::vec4(100.f, 25.f, 25.f, 1.f);
    moon_mesh->hasTexture = true;
    moon_mesh->textures[0] = "moon_texture.bmp";
    moon_mesh->textureRatios[0] = 1.0f;
    meshArray.push_back(moon_mesh);

    // skybox sphere with inverted normals
    sModelDrawInfo skybox_sphere_obj;
    LoadModel(meshFiles[5], skybox_sphere_obj);
    if (!VAOMan->LoadModelIntoVAO("skybox_sphere", skybox_sphere_obj, shaderID)) {
        std::cerr << "Could not load model into VAO" << std::endl;
    }
    skybox_sphere_mesh = new cMeshInfo();
    skybox_sphere_mesh->meshName = "skybox_sphere";
    skybox_sphere_mesh->friendlyName = "skybox_sphere";
    skybox_sphere_mesh->isSkyBoxMesh = true;
    meshArray.push_back(skybox_sphere_mesh);

    // skybox/cubemap textures
    std::cout << "\nLoading Textures";

    std::string errorString = "";
    TextureMan = new cBasicTextureManager();

    TextureMan->SetBasePath("../assets/textures");
    
    const char* skybox_name = "NightSky";
    if (TextureMan->CreateCubeTextureFromBMPFiles("NightSky",
                                                  "SpaceBox_right1_posX.bmp",
                                                  "SpaceBox_left2_negX.bmp",
                                                  "SpaceBox_top3_posY.bmp",
                                                  "SpaceBox_bottom4_negY.bmp",
                                                  "SpaceBox_front5_posZ.bmp",
                                                  "SpaceBox_back6_negZ.bmp",
                                                  true, errorString))
    {
        std::cout << "\nLoaded skybox textures: " << skybox_name << std::endl;
    }
    else 
    {
        std::cout << "\nError: failed to load skybox because " << errorString;
    }

    // Basic texture2D
    if (TextureMan->Create2DTextureFromBMPFile("moon_texture.bmp"))
    {
        std::cout << "Loaded moon texture." << std::endl;
    }
    else 
    {
        std::cout << "Error: failed to load moon texture.";
    }
    
    if (TextureMan->Create2DTextureFromBMPFile("man.bmp"))
    {
        std::cout << "Loaded player texture." << std::endl;
    }
    else 
    {
        std::cout << "Error: failed to load player texture.";
    }

    // reads scene descripion files for positioning and other info
    ReadSceneDescription();
}

void Update() {

    //MVP
    glm::mat4x4 model, view, projection;
    glm::vec3 upVector = glm::vec3(0.f, 1.f, 0.f);

    GLint modelLocaction = glGetUniformLocation(shaderID, "Model");
    GLint viewLocation = glGetUniformLocation(shaderID, "View");
    GLint projectionLocation = glGetUniformLocation(shaderID, "Projection");
    GLint modelInverseLocation = glGetUniformLocation(shaderID, "ModelInverse");
    
    //Lighting
    ManageLights();

    float ratio;
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    ratio = width / (float)height;
    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // mouse support
    if (enableMouse) {
        view = glm::lookAt(cameraEye, cameraEye + cameraTarget, upVector);
        projection = glm::perspective(glm::radians(fov), ratio, 0.1f, 10000.f);
    }
    else {
        view = glm::lookAt(cameraEye, cameraTarget, upVector);
        projection = glm::perspective(0.6f, ratio, 0.1f, 10000.f);
    }

    glm::vec4 viewport = glm::vec4(0, 0, width, height);

    GLint eyeLocationLocation = glGetUniformLocation(shaderID, "eyeLocation");
    glUniform4f(eyeLocationLocation, cameraEye.x, cameraEye.y, cameraEye.z, 1.f);

    currentTime = glfwGetTime();
    timeDiff = currentTime - beginTime;
    frameCount++;

    if (theEditMode == TAKE_CONTROL) {
        cameraEye = player_mesh->position - glm::vec3(35.f, -4.f, 0.f);
        if (!enableMouse) {
            cameraTarget = player_mesh->position;
        }
    }

    bulb_mesh->position = player_mesh->position - glm::vec3(75.f, -25.f, 0.f);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        mouseClick = true;
    }
    else mouseClick = false;

    for (int i = 0; i < meshArray.size(); i++) {

        cMeshInfo* currentMesh = meshArray[i];
        model = glm::mat4x4(1.f);

        if (currentMesh->isVisible == false) {
            continue;
        }

        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.f), currentMesh->position);
        glm::mat4 scaling = glm::scale(glm::mat4(1.f), currentMesh->scale);

        if (currentMesh->isSkyBoxMesh) {
            model = glm::mat4x4(1.f);
        }

        glm::mat4 rotation = glm::mat4(currentMesh->rotation);

        model *= translationMatrix;
        model *= rotation;
        model *= scaling;

        glUniformMatrix4fv(modelLocaction, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

        glm::mat4 modelInverse = glm::inverse(glm::transpose(model));
        glUniformMatrix4fv(modelInverseLocation, 1, GL_FALSE, glm::value_ptr(modelInverse));

        if (currentMesh->isWireframe) 
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        GLint useIsTerrainMeshLocation = glGetUniformLocation(shaderID, "bIsTerrainMesh");

        if (currentMesh->isTerrainMesh) 
        {
            glUniform1f(useIsTerrainMeshLocation, (GLfloat)GL_TRUE);
        }
        else 
        {
            glUniform1f(useIsTerrainMeshLocation, (GLfloat)GL_FALSE);
        }

        GLint RGBAColourLocation = glGetUniformLocation(shaderID, "RGBAColour");

        glUniform4f(RGBAColourLocation, currentMesh->RGBAColour.r, currentMesh->RGBAColour.g, currentMesh->RGBAColour.b, currentMesh->RGBAColour.w);

        GLint useRGBAColourLocation = glGetUniformLocation(shaderID, "useRGBAColour");

        if (currentMesh->useRGBAColour)
        {
            glUniform1f(useRGBAColourLocation, (GLfloat)GL_TRUE);
        }
        else
        {
            glUniform1f(useRGBAColourLocation, (GLfloat)GL_FALSE);
        }
        
        GLint bHasTextureLocation = glGetUniformLocation(shaderID, "bHasTexture");

        if (currentMesh->hasTexture) 
        {
            glUniform1f(bHasTextureLocation, (GLfloat)GL_TRUE);

            std::string texture0 = currentMesh->textures[0];    // moon
 
            GLuint texture0ID = TextureMan->getTextureIDFromName(texture0);

            GLuint texture0Unit = 0;
            glActiveTexture(texture0Unit + GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture0ID);

            GLint texture0Location = glGetUniformLocation(shaderID, "texture0");
            glUniform1i(texture0Location, texture0Unit);

            GLint texRatio_0_3 = glGetUniformLocation(shaderID, "texRatio_0_3");
            glUniform4f(texRatio_0_3,
                        currentMesh->textureRatios[0],
                        currentMesh->textureRatios[1],
                        currentMesh->textureRatios[2],
                        currentMesh->textureRatios[3]);
            
        }
        else 
        {
            glUniform1f(bHasTextureLocation, (GLfloat)GL_FALSE);
        }

        GLint doNotLightLocation = glGetUniformLocation(shaderID, "doNotLight");

        if (currentMesh->doNotLight) 
        {
            glUniform1f(doNotLightLocation, (GLfloat)GL_TRUE);
        }
        else 
        {
            glUniform1f(doNotLightLocation, (GLfloat)GL_FALSE);
        }

        if (theEditMode == TAKE_CONTROL) {
            if (currentMesh->friendlyName == "theAI") {
                currentMesh->rotation = glm::lookAt(currentMesh->position, currentMesh->position + player_mesh->position, upVector);
                std::cout << currentMesh->rotation.x << ", " << currentMesh->rotation.y << ", " << currentMesh->rotation.z << std::endl;
            }
        }
        
        glm::vec3 cursorPos;

        // Division is expensive
        cursorPos.x = width * 0.5;
        cursorPos.y = height * 0.5;

        glm::vec3 worldSpaceCoordinates = glm::unProject(cursorPos, view, projection, viewport);
        
        glm::normalize(worldSpaceCoordinates);
        
        if (mouseClick) {}

        GLint bIsSkyboxObjectLocation = glGetUniformLocation(shaderID, "bIsSkyboxObject");

        if (currentMesh->isSkyBoxMesh) {

            //skybox texture
            GLuint cubeMapTextureNumber = TextureMan->getTextureIDFromName("NightSky");
            GLuint texture30Unit = 30;			// Texture unit go from 0 to 79
            glActiveTexture(texture30Unit + GL_TEXTURE0);	// GL_TEXTURE0 = 33984
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureNumber);
            GLint skyboxTextureLocation = glGetUniformLocation(shaderID, "skyboxTexture");
            glUniform1i(skyboxTextureLocation, texture30Unit);

            glUniform1f(bIsSkyboxObjectLocation, (GLfloat)GL_TRUE);
            currentMesh->position = cameraEye;
            currentMesh->SetUniformScale(7500.f);
        }
        else {
            glUniform1f(bIsSkyboxObjectLocation, (GLfloat)GL_FALSE);
        }
        
        sModelDrawInfo modelInfo;
        if (VAOMan->FindDrawInfoByModelName(meshArray[i]->meshName, modelInfo)) {

            glBindVertexArray(modelInfo.VAO_ID);
            glDrawElements(GL_TRIANGLES, modelInfo.numberOfIndices, GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0);
        }
        else {
            std::cout << "Model not found." << std::endl;
        }

        if (currentMesh->hasChildMeshes) {

            sModelDrawInfo modelInfo;
            if (VAOMan->FindDrawInfoByModelName(currentMesh->vecChildMeshes[0]->meshName, modelInfo)) {

                glBindVertexArray(modelInfo.VAO_ID);
                glDrawElements(GL_TRIANGLES, modelInfo.numberOfIndices, GL_UNSIGNED_INT, (void*)0);
                glBindVertexArray(0);
            }
            else {
                std::cout << "Model not found." << std::endl;
            }
        }
    }

    glfwSwapBuffers(window);
    glfwPollEvents();

    //const GLubyte* vendor = glad_glGetString(GL_VENDOR); // Returns the vendor
    const GLubyte* renderer = glad_glGetString(GL_RENDERER); // Returns a hint to the model

    if (timeDiff >= 1.f / 30.f) {
        std::string frameRate = std::to_string((1.f / timeDiff) * frameCount);
        std::string frameTime = std::to_string((timeDiff / frameCount) * 1000);

        std::stringstream ss;
        ss << " Camera: " << "(" << cameraEye.x << ", " << cameraEye.y << ", " << cameraEye.z << ")"
           << " Target: Index = " << object_index << ", MeshName: " << meshArray[object_index]->friendlyName << ", Position: (" << meshArray[object_index]->position.x << ", " << meshArray[object_index]->position.y << ", " << meshArray[object_index]->position.z << ")"
           << " FPS: " << frameRate << " ms: " << frameTime << " GPU: " << renderer;

        glfwSetWindowTitle(window, ss.str().c_str());

        beginTime = currentTime;
        frameCount = 0;
    }
}

void Shutdown() {

    glfwDestroyWindow(window);
    glfwTerminate();

    window = nullptr;
    delete window;

    exit(EXIT_SUCCESS);
}

void ReadFromFile() {

    std::ifstream readFile("readFile.txt");
    std::string input0;

    while (readFile >> input0) {
        meshFiles.push_back(input0);
        readIndex++;
    }  
}

// All lights managed here
void ManageLights() {
    
    GLint PositionLocation = glGetUniformLocation(shaderID, "sLightsArray[0].position");
    GLint DiffuseLocation = glGetUniformLocation(shaderID, "sLightsArray[0].diffuse");
    GLint SpecularLocation = glGetUniformLocation(shaderID, "sLightsArray[0].specular");
    GLint AttenLocation = glGetUniformLocation(shaderID, "sLightsArray[0].atten");
    GLint DirectionLocation = glGetUniformLocation(shaderID, "sLightsArray[0].direction");
    GLint Param1Location = glGetUniformLocation(shaderID, "sLightsArray[0].param1");
    GLint Param2Location = glGetUniformLocation(shaderID, "sLightsArray[0].param2");

    //glm::vec3 lightPosition0 = meshArray[1]->position;
    glm::vec3 lightPosition0 = meshArray[0]->position;
    glUniform4f(PositionLocation, lightPosition0.x, lightPosition0.y, lightPosition0.z, 1.0f);
    //glUniform4f(PositionLocation, 0.f, 0.f, 0.f, 1.0f);
    glUniform4f(DiffuseLocation, 1.f, 1.f, 1.f, 1.f);
    glUniform4f(SpecularLocation, 1.f, 1.f, 1.f, 1.f);
    glUniform4f(AttenLocation, 0.1f, 0.5f, 0.0f, 1.f);
    glUniform4f(DirectionLocation, 1.f, 1.f, 1.f, 1.f);
    glUniform4f(Param1Location, 0.f, 0.f, 0.f, 1.f); //x = Light Type
    glUniform4f(Param2Location, 1.f, 0.f, 0.f, 1.f); //x = Light on/off
}

//read scene description files
void ReadSceneDescription() {
    std::ifstream File("sceneDescription.txt");
    if (!File.is_open()) {
        std::cerr << "Could not load file." << std::endl;
        return;
    }
    
    int number = 0;
    std::string input0;
    std::string input1;
    std::string input2;
    std::string input3;

    std::string temp;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    File >> number;

    for (int i = 0; i < number; i++) {
        File >> input0                                                         
             >> input1 >> position.x >> position.y >> position.z 
             >> input2 >> rotation.x >> rotation.y >> rotation.z
             >> input3 >> scale.x >> scale.y >> scale.z;

        /*  long_highway
            position 0.0 -1.0 0.0
            rotation 0.0 0.0 0.0
            scale 1.0 1.0 1.0
        */

        temp = input0;

        if (input1 == "position") {
            meshArray[i]->position.x = position.x;
            meshArray[i]->position.y = position.y;
            meshArray[i]->position.z = position.z;
        }
        if (input2 == "rotation") {
            meshArray[i]->AdjustRoationAngleFromEuler(rotation);
        }
        if (input3 == "scale") {
            meshArray[i]->scale.x = scale.x;             
            meshArray[i]->scale.y = scale.y;             
            meshArray[i]->scale.z = scale.z;             
        }
    }
    File.close();

    std::string yes;
    float x, y, z;
    std::ifstream File1("cameraEye.txt");
    if (!File1.is_open()) {
        std::cerr << "Could not load file." << std::endl;
        return;
    }
    while (File1 >> yes >> x >> y >> z) {
        ::cameraEye.x = x;
        ::cameraEye.y = y;
        ::cameraEye.z = z;
    }
    File1.close();
}

float RandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

bool RandomizePositions(cMeshInfo* mesh) {

    int i = 0;
    float x, y, z, w;

    x = RandomFloat(-500, 500);
    y = mesh->position.y;
    z = RandomFloat(-200, 200);

    mesh->position = glm::vec3(x, y, z);
    
    return true;
}

int main(int argc, char** argv) {

    Initialize();
    Render();

    while (!glfwWindowShouldClose(window)) {
        Update();
    }

    Shutdown();

    return 0;
}