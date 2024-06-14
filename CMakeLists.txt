cmake_minimum_required(VERSION 3.28)
project(ICP_Project)

set(CMAKE_CXX_STANDARD 17)

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# GLEW
set(GLEW_INCLUDE_DIR "/opt/homebrew/Cellar/glew/2.2.0_1/include")
set(GLEW_LIBRARY "/opt/homebrew/Cellar/glew/2.2.0_1/lib/libGLEW.dylib")
include_directories(${GLEW_INCLUDE_DIR})

# GLFW
find_package(glfw3 REQUIRED)

# GLM
find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

add_executable(ICP_Project main.cpp
        App.cpp
        App.h
        Mesh.cpp
        Mesh.h
        ShaderProgram.cpp
        ShaderProgram.h
        OBJloader.cpp
        OBJloader.h
        Camera.cpp
        Camera.h
        Game.cpp
        Game.h
        GameCallbacks.cpp
        GameInit.cpp
        Vertex.h
        Texture.cpp
        Texture.h
        Map.cpp
        Map.h
        RedColourTracker.cpp
        RedColourTracker.h
        synced_dequeue.cpp
        synced_dequeue.h
)
target_link_libraries(ICP_Project ${OpenCV_LIBS} ${GLEW_LIBRARY} glfw "-framework OpenGL")
