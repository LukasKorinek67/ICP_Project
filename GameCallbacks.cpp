#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Game.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <unordered_set>


bool vsync = true;
bool fscreen = false;

// Error callback
void Game::error_callback(int error, const char* description) {
    std::cerr << "GLFW error: " << description << std::endl;
}

// Tato funkce se stará o to, aby aplikace správnì reagovala na zmìnu velikosti okna a správný pøepoèet všech dùležitých parametrù pro vykreslování
void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    auto inst = static_cast<Game*>(glfwGetWindowUserPointer(window));
    inst->width = width;
    inst->height = height;
    glViewport(0, 0, width, height);
    inst->update_projection_matrix();
    inst->shader->setUniform("uPm", inst->projection_matrix);
    std::cout << "window size changed. width: " << width << " height: " << height << "\n";
}

// Nastavení callbackù pro klávesy
void Game::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto inst = static_cast<Game*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        if (fscreen) {
            glfwSetWindowMonitor(window, NULL, 0, 0, 640, 480, NULL);
            fscreen = false;
        }
        else {
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
            glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
            fscreen = true;
        }
        return;
    }
    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        glfwSwapInterval(vsync);
        std::cout << "vsync set: " << vsync << "\n";
        vsync = !vsync;
        return;
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        if (inst->driveMode) {
            inst->last_drive_camera = inst->camera;
        }
        else {
            inst->camera = inst->last_drive_camera;
        }
        inst->driveMode = !inst->driveMode;
        std::cout << "Drive mode: " << inst->driveMode << "\n";
        return;
    }
    if (action == GLFW_PRESS) {
        inst->keys.insert(key);
    }
    else if (action == GLFW_RELEASE)
        inst->keys.erase(key);
}

void Game::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
        //auto inst = static_cast<Game*>(glfwGetWindowUserPointer(window));
        std::cout << "Mouse click!\n";
    }
    if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS) {
        //auto inst = static_cast<Game*>(glfwGetWindowUserPointer(window));
        std::cout << "Right mouse click!\n";
    }
}
void Game::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    auto inst = static_cast<Game*>(glfwGetWindowUserPointer(window));
    inst->camera.ProcessMouseMovement(xpos - inst->xpos_prev, (ypos - inst->ypos_prev) * -1, GL_TRUE, inst->driveMode);
    inst->xpos_prev = xpos;
    inst->ypos_prev = ypos;
}
void Game::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    auto inst = static_cast<Game*>(glfwGetWindowUserPointer(window));
    inst->fov -= 10.0f * yoffset;
    inst->fov = std::clamp(inst->fov, 20.0f, 170.0f);
    inst->update_projection_matrix();
    inst->shader->setUniform("uPm", inst->projection_matrix);
    std::cout << "scroll event. xoffset: " << xoffset << " yoffset: " << yoffset << "\n";
}

// Funkce bere komplexní informace o OpenGL debug zprávì a pøevádí je na lidsky èitelný formát, který je následnì vypsán do konzole
// Umožòuje snadno sledovat a diagnostikovat problémy v aplikaci
void GLAPIENTRY Game::MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    auto const src_str = [source]() {
        switch (source)
        {
        case GL_DEBUG_SOURCE_API: return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
        case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
        case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
        case GL_DEBUG_SOURCE_OTHER: return "OTHER";
        default: return "Unknown";
        }
        }();

        auto const type_str = [type]() {
            switch (type)
            {
            case GL_DEBUG_TYPE_ERROR: return "ERROR";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
            case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
            case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
            case GL_DEBUG_TYPE_MARKER: return "MARKER";
            case GL_DEBUG_TYPE_OTHER: return "OTHER";
            default: return "Unknown";
            }
            }();

            auto const severity_str = [severity]() {
                switch (severity) {
                case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
                case GL_DEBUG_SEVERITY_LOW: return "LOW";
                case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
                case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
                default: return "Unknown";
                }
                }();

                std::cout << "[GL CALLBACK]: " <<
                    "source = " << src_str <<
                    ", type = " << type_str <<
                    ", severity = " << severity_str <<
                    ", ID = '" << id << '\'' <<
                    ", message = '" << message << '\'' << std::endl;
}