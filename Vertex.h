#pragma once
#include <glm/glm.hpp>

// Popis jednoho vrcholu ve 3D
struct vertex {
    glm::vec3 position;
    glm::vec2 texCoord; //texturov� sou�adnice
    glm::vec3 normal;   //norm�lov� vektor - sv�tlo
};
