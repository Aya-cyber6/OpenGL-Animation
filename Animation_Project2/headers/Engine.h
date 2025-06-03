#ifndef ENGINE_H
#define ENGINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "..//headers/Camera.h"
#include "..//headers/Mesh.h"
#include "..//headers/SkinnedMesh.h"
#include "..//headers/SkinningTechnique.h"
#include <chrono>



class Engine {
public:
    Engine();
    ~Engine();

    bool Init(GLFWwindow* window);

    void RenderSceneCB();
    void KeyboardCB(unsigned char key, int mouse_x, int mouse_y);
    void SpecialKeyboardCB(int key, int mouse_x, int mouse_y);
    void PassiveMouseCB(int x, int y);

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

    int WINDOW_WIDTH;
    int WINDOW_HEIGHT;
    GLFWwindow* m_Window;
};

#endif // ENGINE_H
