
#include <glad/glad.h>
#include "..//headers/Engine.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/gtx/string_cast.hpp>


#define COLOR_TEXTURE_UNIT_INDEX        0
#define SPECULAR_EXPONENT_UNIT_INDEX       8



Engine::Engine() {

    GLclampf Red = 0.5f, Green = 0.3f, Blue = 0.6f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);

    float FOV = 45.0f;
    float zNear = 0.1f;
    float zFar = 100.0f;

    persProjInfo = { FOV, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT, zNear, zFar };

    pointLights[0].AmbientIntensity = 1.0f;
    pointLights[0].DiffuseIntensity = 1.0f;
    pointLights[0].Color = glm::vec3(1.0f, 1.0f, 0.0f);
    pointLights[0].Attenuation.Linear = 0.0f;
    pointLights[0].Attenuation.Exp = 0.0f;

    pointLights[1].DiffuseIntensity = 0.0f;
    pointLights[1].Color = glm::vec3(0.0f, 1.0f, 1.0f);
    pointLights[1].Attenuation.Linear = 0.0f;
    pointLights[1].Attenuation.Exp = 0.2f;

    spotLights[0].DiffuseIntensity = 1.0f;
    spotLights[0].Color = glm::vec3(1.0f, 1.0f, 1.0f);
    spotLights[0].Attenuation.Linear = 0.01f;
    spotLights[0].Cutoff = 20.0f;

    spotLights[1].DiffuseIntensity = 1.0f;
    spotLights[1].Color = glm::vec3(1.0f, 1.0f, 0.0f);
    spotLights[1].Attenuation.Linear = 0.01f;
    spotLights[1].Cutoff = 30.0f;
}

Engine::~Engine()
{
    if (pGameCamera) {
        delete pGameCamera;
    }

    if (pMesh1) {
        delete pMesh1;
    }
}


bool Engine::Init() {

    glm::vec3 CameraPos(0.0f, 0.0f, 5.0f);
    glm::vec3 CameraTarget(0.0f, 0.0f, 0.0f);
    glm::vec3 CameraUp(0.0f, 1.0f, 0.0f);

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, CameraPos, CameraTarget, CameraUp);
    pMesh1 = new SkinnedMesh();

    if (!pMesh1->LoadMesh("res/donut/donut.obj")) {
        printf("Mesh load failedddddddddd\n");
        return false;
    }
    //////////  SKININNG PART  //////////////////

    pSkinningTech = new SkinningTechnique();

    if (!pSkinningTech->Init())  // create a shader program initialize it and setup the uniforms and lightining variables
    {
        return false;
    }

    pSkinningTech->Enable();  //  glUseProgram(m_shaderProg);

    pSkinningTech->SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    pSkinningTech->SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);
    pSkinningTech->SetDisplayBoneIndex(DisplayBoneIndex);


    return true;
}


//void Engine::Update(float deltaTime) {
    // TODO: Handle character logic, camera movement, user input, and branching decisions}

void Engine::RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    pGameCamera->OnRender();

    glm::mat4 World = glm::mat4(1.0f);  // No translate, no rotate, no scale


    // Get View and Projection from your camera and projection info (assuming compatible types)
    glm::mat4 View = pGameCamera->GetMatrix();
    glm::mat4 Projection;
    // Replace your InitPersProjTransform with glm::perspective or your own function

    float aspect;
    if (persProjInfo.Height == 0.0f) {
        std::cerr << "[ERROR] Projection height is zero. Setting fallback height = 1.\n";
        aspect = 1.0f;
    }
    else {
        aspect = persProjInfo.Width / persProjInfo.Height;
    }

    Projection = glm::perspective(
        glm::radians(persProjInfo.FOV),
        persProjInfo.Width / persProjInfo.Height,
        persProjInfo.zNear,
        persProjInfo.zFar
    );

    glm::mat4 WVP = Projection * View * World;
    pSkinningTech->SetWVP(WVP);

    // For lights, convert their world positions/directions to local space of the mesh:
    glm::mat4 invWorld = glm::inverse(World);

    // Point Lights
    pointLights[0].WorldPosition = glm::vec3(0.0f, 1.0f, 1.0f);
    glm::vec4 localPos = invWorld * glm::vec4(pointLights[0].WorldPosition, 1.0f);
    pointLights[0].LocalPosition = glm::vec3(localPos);

    pointLights[1].WorldPosition = glm::vec3(10.0f, 1.0f, 0.0f);
    localPos = invWorld * glm::vec4(pointLights[1].WorldPosition, 1.0f);
    pointLights[1].LocalPosition = glm::vec3(localPos);

    pSkinningTech->SetPointLights(2, pointLights);

    // Spot Lights
    spotLights[0].WorldPosition = pGameCamera->GetPosition();
    spotLights[0].WorldDirection = pGameCamera->GetTarget();

    localPos = invWorld * glm::vec4(spotLights[0].WorldPosition, 1.0f);
    glm::vec4 localDir = invWorld * glm::vec4(spotLights[0].WorldDirection, 0.0f);

    spotLights[0].LocalPosition = glm::vec3(localPos);
    spotLights[0].LocalDirection = glm::normalize(glm::vec3(localDir));

    spotLights[1].WorldPosition = glm::vec3(0.0f, 1.0f, 0.0f);
    spotLights[1].WorldDirection = glm::vec3(0.0f, -1.0f, 0.0f);

    localPos = invWorld * glm::vec4(spotLights[1].WorldPosition, 1.0f);
    localDir = invWorld * glm::vec4(spotLights[1].WorldDirection, 0.0f);

    spotLights[1].LocalPosition = glm::vec3(localPos);
    spotLights[1].LocalDirection = glm::normalize(glm::vec3(localDir));

    pSkinningTech->SetSpotLights(2, spotLights);

    // Material & camera local position
    pSkinningTech->SetMaterial(pMesh1->GetMaterial());

    glm::vec4 camLocalPos4 = invWorld * glm::vec4(pGameCamera->GetPosition(), 1.0f);
    glm::vec3 camLocalPos3 = glm::vec3(camLocalPos4);
    pSkinningTech->SetCameraLocalPos(camLocalPos3);


    pMesh1->Render();


}


#define ATTEN_STEP 0.01f

#define ANGLE_STEP 1.0f


void Engine::ProcessInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true); // Close window on ESC

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        ProcessKey(GLFW_KEY_W, GLFW_PRESS, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        ProcessKey(GLFW_KEY_S, GLFW_PRESS, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        ProcessKey(GLFW_KEY_A, GLFW_PRESS, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        ProcessKey(GLFW_KEY_D, GLFW_PRESS, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        ProcessKey(GLFW_KEY_SPACE, GLFW_PRESS, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        ProcessKey(GLFW_KEY_LEFT_SHIFT, GLFW_PRESS, deltaTime);
}


void Engine::ProcessKey(int key, int action, float deltaTime) {
        //outside switch
        if (pGameCamera) {
            pGameCamera->OnKeyboard(key, deltaTime);
        }
    }

void Engine::ProcessMouse(double xpos, double ypos) {
    if (pGameCamera) {
        pGameCamera->OnMouse(static_cast<int>(xpos), static_cast<int>(ypos));
    }
}


void Engine::MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) {
        engine->ProcessMouse(xpos, ypos);
    }
}