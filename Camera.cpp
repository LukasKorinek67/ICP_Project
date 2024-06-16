#include "Camera.h"

Camera::Camera(glm::vec3 position) :Position(position) {
    this->Up = glm::vec3(0.0f, 1.0f, 0.0f);
    this->updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
}

glm::vec3 Camera::ProcessMovement(std::unordered_set<int> keys, GLfloat deltaTime, bool driveMode) {
    auto window = glfwGetCurrentContext();
    glm::vec3 movement = { 0.0f, 0.0f, 0.0f };
    for (const int& key : keys) {
        switch (key) {
        case GLFW_KEY_UP:
        case GLFW_KEY_W:
            movement += ProcessInput(Camera::direction::FORWARD, deltaTime);
            break;

        case GLFW_KEY_DOWN:
        case GLFW_KEY_S:
            movement += ProcessInput(Camera::direction::BACKWARD, deltaTime);
            break;

        case GLFW_KEY_LEFT:
        case GLFW_KEY_A:
            if (driveMode && isCarInMovement(keys)) {
                if (isCarMovingForward(keys)) {
                    processCameraRollMovement(-turning_power, 0.0f, GL_TRUE);
                }
                else {
                    processCameraRollMovement(turning_power, 0.0f, GL_TRUE);
                }
            }
            else if (!driveMode) {
                movement += ProcessInput(Camera::direction::LEFT, deltaTime);
            }
            break;

        case GLFW_KEY_RIGHT:
        case GLFW_KEY_D:
            if (driveMode && isCarInMovement(keys)) {
                if (isCarMovingForward(keys)) {
                    processCameraRollMovement(turning_power, 0.0f, GL_TRUE);
                }
                else {
                    processCameraRollMovement(-turning_power, 0.0f, GL_TRUE);
                }
            }
            else if (!driveMode) {
                movement += ProcessInput(Camera::direction::RIGHT, deltaTime);
            }
            break;

        case GLFW_KEY_SPACE:
            if (isCarMovingForward(keys)) {
                movement += ProcessInput(Camera::direction::FORWARD, 2 * deltaTime);
            }
            break;
        case GLFW_KEY_P:
            movement += ProcessInput(Camera::direction::UP, deltaTime);
            break;
        case GLFW_KEY_L:
            movement += ProcessInput(Camera::direction::DOWN, deltaTime);
            break;
        default:
            break;
        }
    }
    return movement;
}

bool Camera::isCarInMovement(std::unordered_set<int> keys) {
    for (const int& key : keys) {
        if (key == GLFW_KEY_W || key == GLFW_KEY_S || key == GLFW_KEY_UP || key == GLFW_KEY_DOWN) {
            return true;
        }
    }
    return false;
}

bool Camera::isCarMovingForward(std::unordered_set<int> keys) {
    for (const int& key : keys) {
        if (key == GLFW_KEY_W || key == GLFW_KEY_UP) {
            return true;
        }
    }
    return false;
}

glm::vec3 Camera::ProcessInput(Camera::direction direction, GLfloat deltaTime) {
    GLfloat velocity = this->MovementSpeed * deltaTime;
    glm::vec3 dir;
    switch (direction) {
    case direction::FORWARD:
        //dir = this->Front * velocity;
        dir.x = cos(glm::radians(this->Yaw));
        dir.z = sin(glm::radians(this->Yaw));
        dir.y = 0;
        dir = dir * velocity;
        return dir;
        //return this->Front * velocity;
    case direction::BACKWARD:
        //dir = -this->Front * velocity;
        dir.x = -cos(glm::radians(this->Yaw));
        dir.z = -sin(glm::radians(this->Yaw));
        dir.y = 0;
        dir = dir * velocity;
        return dir;
        //return -this->Front * velocity;
    case direction::LEFT:
        dir = -this->Right * velocity;
        dir.y = 0;
        return dir;
        //return -this->Right * velocity;
    case direction::RIGHT:
        dir = this->Right * velocity;
        dir.y = 0;
        return dir;
        //return this->Right * velocity;
    case direction::UP:
        return this->Up * velocity;
    case direction::DOWN:
        return -this->Up * velocity;
    }

    return glm::vec3(0.0f);
}

void Camera::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch, bool driveMode) {
    if (!driveMode) {
        processCameraRollMovement(xoffset, yoffset, constraintPitch, driveMode);
    }
}

void Camera::processCameraRollMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch, bool driveMode) {
    xoffset *= this->MouseSensitivity;
    yoffset *= this->MouseSensitivity;

    this->Yaw += xoffset;
    this->Pitch += yoffset;

    if (constraintPitch) {
        if (this->Pitch > 89.0f)
            this->Pitch = 89.0f;
        if (this->Pitch < -89.0f)
            this->Pitch = -89.0f;
    }

    if (driveMode) {
        this->Pitch = -10.0f;
    }
    this->updateCameraVectors();
}

void Camera::horizontalCameraMovement(GLfloat xoffset, GLboolean constraintPitch, bool driveMode) {
    xoffset *= this->MouseSensitivity;
    this->Yaw += xoffset;
}

void Camera::verticalCameraMovement(GLfloat yoffset, GLboolean constraintPitch, bool driveMode) {
    yoffset *= this->MouseSensitivity;
    this->Pitch += yoffset;
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
    front.y = sin(glm::radians(this->Pitch));
    front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));

    this->Front = glm::normalize(front);
    this->Right = glm::normalize(glm::cross(this->Front, glm::vec3(0.0f, 1.0f, 0.0f)));
    this->Up = glm::normalize(glm::cross(this->Right, this->Front));
}