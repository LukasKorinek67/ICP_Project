#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <chrono>
#include <stack>
#include <random>
#include <numeric>
#include <vector>

// OpenCV
#include <opencv2/opencv.hpp>

// OpenGL Extension Wrangler
#include <GL/glew.h>
//#include <GL/wglew.h> //WGLEW = Windows GL Extension Wrangler (change for different platform)

// GLFW toolkit
#include <GLFW/glfw3.h>

// OpenGL math
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Mesh.h"
#include "Camera.h"
#include "synced_dequeue.h"


class Game {
public:
    Game();
    ~Game();
    bool init(void);
    int run(synced_deque<bool>& fronta, bool red_tracker_on);
    std::unordered_set<int> keys;
    double xpos_prev = 0;
    double ypos_prev = 0;
    bool driveMode;
private:
    GLFWwindow* window = { nullptr };
    int swap_interval = 1;
    int width{ 0 }, height{ 0 };
    Camera camera = Camera(glm::vec3(0.0, 0.0, 10.0));
    Camera last_drive_camera = Camera(glm::vec3(0.0, 0.0, 10.0));
    float fov = 60.0f;
    ShaderProgram *shader;
    glm::mat4 projection_matrix;
    std::unordered_map<std::string, Mesh> scene;

    // Init funkce
    void init_glew(void);
    void init_glfw(void);
    void init_gl_debug();
    void init_assets(void);

    // Print info funkce
    void print_startup_info();
    void print_glfw_info(void);
    void print_glm_info();
    void print_gl_info();

    // Callbacks
    void set_callbacks();
    static void error_callback(int error, const char* description);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

    double last_frame_time=0;
    void handle_player_movement();
    glm::vec3 calculate_collisions(glm::vec3 position, glm::vec3 offset, std::unordered_map<std::string, Mesh> &scene);
    glm::vec3 calculate_formula_collisions(glm::vec3 position, glm::vec3 offset, std::unordered_map<std::string, Mesh> &scene);
    bool check_collision(glm::vec3 position, glm::vec3 offset, Mesh object, bool middleInZeros);
    bool check_collision_with_formula(glm::vec3 position, glm::vec3 offset, Mesh object, bool middleInZeros);
    bool isPointInRectangle(const glm::vec2& point, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& d);
    void update_dynamic_models();
    void draw_all_models();
    void set_lights();
    void handle_semafor_change(synced_deque<bool>& queue, bool red_tracker_on);
    glm::vec3 get_formula_position_coordinates();
    void set_formula_model_position();

    void update_projection_matrix(void);

    glm::vec3 pointLightPositions[3] = {
        glm::vec3(50.0f,  8.0f,  50.0f),
        glm::vec3(25.0f, 3.0f, 25.0f),
        glm::vec3(87.6f, 2.7f, 41.0f),  //semafor
    };
    bool red_light_on_semafor = false;


    // Drones
    int currentTrianglePoint;
    glm::vec3 drone1Position;
    glm::vec3 drone1TargetPositions[3];
    float droneSpeed;

    // Wheel
    glm::vec3 wheelPosition;
};

#endif //GAME_H
