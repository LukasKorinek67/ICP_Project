#pragma once
#include <GL/glew.h>
// #include <GL/wglew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <unordered_set>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

class Camera
{
public:
    enum class direction
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Right;
    glm::vec3 Up; 

    GLfloat Yaw = -90.0f;
    GLfloat Pitch = -10.0f;
    GLfloat Roll = 0.0f;

    // Camera options
    GLfloat MovementSpeed = 25.0f; // L - 10.0f
    GLfloat MouseSensitivity = 0.25f;

    // Turning
    GLfloat turning_power = 15.0f; //L - 4.0f;

    Camera(glm::vec3 position);
    glm::mat4 GetViewMatrix();
    glm::vec3 ProcessMovement(std::unordered_set<int> keys, GLfloat deltaTime, bool driveMode = true);
    bool isCarInMovement(std::unordered_set<int> keys);
    bool isCarMovingForward(std::unordered_set<int> keys);
    glm::vec3 ProcessInput(Camera::direction direction, GLfloat deltaTime);
    void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch = GL_TRUE, bool driveMode = true);
    void processCameraRollMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch = GL_TRUE, bool driveMode = true);
    void horizontalCameraMovement(GLfloat xoffset, GLboolean constraintPitch = GL_TRUE, bool driveMode = true);
    void verticalCameraMovement(GLfloat yoffset, GLboolean constraintPitch = GL_TRUE, bool driveMode = true);

private:
    void updateCameraVectors();
};
