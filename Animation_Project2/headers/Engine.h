#ifndef ENGINE_H
#define ENGINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "..//headers/Camera.h"
#include "..//headers/SkinnedMesh.h"
#include "..//headers/SkinningTechnique.h"
#include <chrono>



class Engine {
public:
    Engine();
    ~Engine();

    bool Init();

    void RenderSceneCB();

    // 🔑 Add these static methods:
    static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
    void ProcessInput(GLFWwindow* window, float deltaTime);

    struct PersProjInfo
    {
        float FOV = 0.0f;
        float Width = 0.0f;
        float Height = 0.0f;
        float zNear = 0.0f;
        float zFar = 0.0f;
    };

private:
    GLuint WVPLocation;
    GLuint SamplerLocation;
    Camera* pGameCamera = NULL;
    SkinnedMesh* pMesh1 = NULL;
    PersProjInfo persProjInfo;
    SkinningTechnique* pSkinningTech = NULL;
    PointLight pointLights[SkinningTechnique::MAX_POINT_LIGHTS];
    SpotLight spotLights[SkinningTechnique::MAX_SPOT_LIGHTS];
    long long StartTime = 0;
    int DisplayBoneIndex = 0;

    int WINDOW_WIDTH = 1280;
    int WINDOW_HEIGHT = 720;
    GLFWwindow* m_Window;


    void ProcessKey(int key, int action, float deltaTime);
    void ProcessMouse(double xpos, double ypos);

};

#endif // ENGINE_H
