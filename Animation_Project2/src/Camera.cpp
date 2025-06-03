#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>     // for glm::rotate
#include <iostream>
#include "..//headers/Camera.h"

static float EDGE_STEP = 1.0f;

Camera::Camera(int windowWidth, int windowHeight)
    : m_windowWidth(windowWidth), m_windowHeight(windowHeight),
      m_position(glm::vec3(0.0f, 0.0f, 3.0f)),
      m_front(glm::vec3(0.0f, 0.0f, -1.0f)),
      m_up(glm::vec3(0.0f, 1.0f, 0.0f)),
      m_worldUp(m_up),
      m_yaw(-90.0f), // Facing towards -Z initially
      m_pitch(0.0f),
      m_mouseSensitivity(0.1f),
      m_speed(2.5f),
      m_firstMouse(true),
      m_lastX(windowWidth / 2.0f),
      m_lastY(windowHeight / 2.0f)
{
    UpdateCameraVectors();
}

Camera::Camera(int windowWidth, int windowHeight, const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
    : m_windowWidth(windowWidth), m_windowHeight(windowHeight),
      m_position(position),
      m_front(glm::normalize(target - position)),
      m_worldUp(up),
      m_mouseSensitivity(0.1f),
      m_speed(2.5f),
      m_firstMouse(true),
      m_lastX(windowWidth / 2.0f),
      m_lastY(windowHeight / 2.0f)
{
    // Calculate yaw and pitch from initial front vector
    m_yaw = glm::degrees(atan2(m_front.z, m_front.x)) - 90.0f;
    m_pitch = glm::degrees(asin(m_front.y));
    UpdateCameraVectors();
}

void Camera::SetPosition(float x, float y, float z) {
    m_position = glm::vec3(x, y, z);
    UpdateCameraVectors();
}

void Camera::OnKeyboard(int key, float deltaTime) {
    float velocity = m_speed * deltaTime;

    if (key == GLFW_KEY_W)
        m_position += m_front * velocity;
    if (key == GLFW_KEY_S)
        m_position -= m_front * velocity;
    if (key == GLFW_KEY_A)
        m_position -= m_right * velocity;
    if (key == GLFW_KEY_D)
        m_position += m_right * velocity;
    if (key == GLFW_KEY_SPACE)
        m_position += m_worldUp * velocity;
    if (key == GLFW_KEY_LEFT_SHIFT)
        m_position -= m_worldUp * velocity;
}

void Camera::OnMouse(float xpos, float ypos)
{
    if (m_firstMouse)
    {
        m_lastX = xpos;
        m_lastY = ypos;
        m_firstMouse = false;
    }

    float xoffset = xpos - m_lastX;
    float yoffset = m_lastY - ypos; // reversed since y-coordinates go bottom to top

    m_lastX = xpos;
    m_lastY = ypos;

    xoffset *= m_mouseSensitivity;
    yoffset *= m_mouseSensitivity;

    m_yaw += xoffset;
    m_pitch += yoffset;

    // Constrain the pitch so the screen doesn't flip
    if (m_pitch > 89.0f)
        m_pitch = 89.0f;
    if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    UpdateCameraVectors();
}

void Camera::OnRender()
{
    bool ShouldUpdate = false;

    if (m_OnLeftEdge) {
        m_AngleH -= EDGE_STEP;
        ShouldUpdate = true;
    }
    else if (m_OnRightEdge) {
        m_AngleH += EDGE_STEP;
        ShouldUpdate = true;
    }

    if (m_OnUpperEdge) {
        if (m_AngleV > -90.0f) {
            m_AngleV -= EDGE_STEP;
            ShouldUpdate = true;
        }
    }
    else if (m_OnLowerEdge) {
        if (m_AngleV < 90.0f) {
            m_AngleV += EDGE_STEP;
            ShouldUpdate = true;
        }
    }

    if (ShouldUpdate) {
        Update();
    }
}


void Camera::Update()
{
    glm::vec3 Yaxis(0.0f, 1.0f, 0.0f);

    // Initial view direction
    glm::vec3 View(1.0f, 0.0f, 0.0f);

    // Rotate the view vector by the horizontal angle around the vertical axis
    View = glm::rotate(View, glm::radians(m_AngleH), Yaxis);
    View = glm::normalize(View);

    // Get horizontal axis (perpendicular to Y and View)
    glm::vec3 U = glm::cross(Yaxis, View);
    U = glm::normalize(U);

    // Rotate view vector by vertical angle around the U axis
    View = glm::rotate(View, glm::radians(m_AngleV), U);

    m_target = glm::normalize(View);

    m_up = glm::normalize(glm::cross(m_target, U));
}

void Camera::UpdateCameraVectors()
{
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);

    // Recalculate Right and Up vector
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));
}

glm::mat4 Camera::GetMatrix() const
{
    return glm::lookAt(
        m_position,          // Camera position
        m_target, // Target position (where camera looks at)
        m_up                 // Up vector
    );
}
