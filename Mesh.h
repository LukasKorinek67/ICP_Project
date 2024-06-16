#pragma once

#include "ShaderProgram.h"
#include "Vertex.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Mesh = reprezentace 3D objektu ve scénì
class Mesh {
public:
	glm::vec3 position;
	float width;
	float height;
	glm::vec3 color = { 1.0,1.0,1.0 };
	GLuint texture_id = 0;
	std::string obj_path;
	ShaderProgram shader;
	Mesh(void);
	Mesh(std::string obj_path, ShaderProgram shader, GLuint texture_id, float texture_scale = 1.0f, float width = 0, float height = 0, glm::vec3 position = glm::vec3(0.0));
	void setModelMatrix(glm::mat4 modelMatrix);
	glm::mat4 getModelMatrix(void);
	void draw(const glm::mat4& M, const glm::mat4& V, const glm::mat4& P, Camera& camera);
	void draw();
private:
	std::vector<vertex> vertices;
	std::vector<GLuint> indices;
	GLuint VAO = 0;	// Vertex Array Object
	GLuint VBO = 0;	// Vertex Buffer Object
	GLuint EBO = 0;	// Element Buffer Object
	GLenum primitive = GL_TRIANGLES;	
	glm::mat4 modelMatrix;
	glm::mat3 modelNormalMatrix;
};
