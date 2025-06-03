#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
    Camera(int windowWidth, int windowHeight);
    Camera(int windowWidth, int windowHeight, const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);

    void SetPosition(float x, float y, float z);
    void OnKeyboard(int key, float deltaTime); // key from GLFW_KEY_*, deltaTime passed in from main loop
    void OnMouse(float xpos, float ypos);
    void OnRender();

    glm::mat4 GetMatrix() const;

    const glm::vec3& GetPosition() const { return m_position; }
    const glm::vec3& GetFront() const { return m_front; }
    const glm::vec3& GetTarget() { return m_target; }

private:
    void Update();
    void UpdateCameraVectors();

    glm::vec3 m_target;
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_worldUp;

    float m_yaw;
    float m_pitch;

    float m_mouseSensitivity;
    float m_speed = 1.0f;

    int m_windowWidth;
    int m_windowHeight;

    float m_AngleH;
    float m_AngleV;

    float m_lastX;
    float m_lastY;
    bool m_firstMouse;

    bool m_OnUpperEdge;
    bool m_OnLowerEdge;
    bool m_OnLeftEdge;
    bool m_OnRightEdge;
};

#endif // CAMERA_H
