
#include <glad/glad.h> // GLAD first
#include <GLFW/glfw3.h>
#include <iostream>

#include "..//headers/Engine.h"

bool IsGLVersionHigher(int major, int minor)
{
    const char* versionStr = (const char*)glGetString(GL_VERSION);
    if (!versionStr) return false;

    int glMajor = 0, glMinor = 0;
    sscanf_s(versionStr, "%d.%d", &glMajor, &glMinor);

    if (glMajor > major) return true;
    if (glMajor == major && glMinor >= minor) return true;
    return false;
}

float deltaTime = 0.0f;
float lastFrame = 0.0f;


int main() {

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Adjust as needed
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Skeletal Animation Project", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);


    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    Engine* engine = new Engine();

    if (!engine->Init()) {
        std::cerr << "Engine initialization failed.\n";
        return -1;
    }

    glfwSetWindowUserPointer(window, engine);
    glfwSetCursorPosCallback(window, Engine::MouseCallback);


    while (!glfwWindowShouldClose(window)) {

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        engine->RenderSceneCB();
        engine->ProcessInput(window, deltaTime);

        //engine.Update(currentTime);
        //engine.Render();
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}