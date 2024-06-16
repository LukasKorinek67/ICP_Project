#pragma once
#include <glm/glm.hpp>

// Popis jednoho vrcholu ve 3D
struct vertex {
    glm::vec3 position;
    glm::vec2 texCoord; //texturové souøadnice
    glm::vec3 normal;   //normálový vektor - svìtlo
};
