#include "Game.h"

#include <iostream>
#include <filesystem>
#include <vector>
#include <opencv2/opencv.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>


int Game::run(synced_deque<bool>& queue, bool red_tracker_on) {
    std::cout << "Game started!\n";

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {
        // clear canvas
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    	handle_player_movement();
    	update_dynamic_models();
    	set_lights();
    	draw_all_models();
    	handle_semafor_change(queue, red_tracker_on);
    	glfwSwapBuffers(window);
    	glfwPollEvents();
    }
    glfwTerminate();
	return 0;
}

void Game::handle_player_movement() {
	double nowGLFW = glfwGetTime();
	double delta_t = nowGLFW - last_frame_time;

	glm::vec3 offset = camera.ProcessMovement(keys, delta_t, driveMode);
	if(driveMode) {
		camera.Position += calculate_formula_collisions(camera.Position, offset, scene);
	} else {
		camera.Position += calculate_collisions(camera.Position, offset, scene);
	}
	last_frame_time = nowGLFW;
}

glm::vec3 Game::calculate_formula_collisions(glm::vec3 position, glm::vec3 offset, std::unordered_map<std::string, Mesh> &scene) {
	bool middleInZeros;
	for (auto& model : scene) {
		if (model.first != "formula") {
			if(model.first == "surface") {
				middleInZeros = false;
			} else {
				middleInZeros = true;
			}
			if (check_collision_with_formula(position, offset, model.second, middleInZeros)) {
				return glm::vec3(0.0);
			}
		}
	}
	return offset;
}

bool Game::check_collision_with_formula(glm::vec3 position, glm::vec3 offset, Mesh object, bool middleInZeros) {
	position = get_formula_position_coordinates();
	glm::vec3 futurePosition = position + offset;
	futurePosition.y = (scene["formula"].height/2) + 0.1f;
	GLfloat formulaWidth = 5.1f;
	GLfloat formulaLength = 1.95f;
	GLfloat formulaHeight = scene["formula"].height - 0.2f;

	glm::vec3 center = futurePosition;
	float yaw = abs((int)camera.Yaw % 360);
	glm::vec3 corners[4];
	// Rotační matice pro otočení o yaw stupňů kolem osy Y
	glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 halfExtents = glm::vec3(formulaWidth / 2, 0, formulaLength / 2);
	glm::vec3 points[4] = {
		center + glm::vec3(-halfExtents.x, 0, -halfExtents.z),
		center + glm::vec3(halfExtents.x, 0, -halfExtents.z),
		center + glm::vec3(halfExtents.x, 0, halfExtents.z),
		center + glm::vec3(-halfExtents.x, 0, halfExtents.z)
	};

	for (int i = 0; i < 4; ++i) {
		glm::vec4 rotatedPoint = transform * glm::vec4(points[i] - center, 1.0f);
		corners[i] = glm::vec3(rotatedPoint) + center;
	}

	glm::vec2 formulaLeftRear(corners[0].x, corners[0].z);
	glm::vec2 formulaRightRear(corners[3].x, corners[3].z);
	glm::vec2 formulaRightFront(corners[2].x, corners[2].z);
	glm::vec2 formulaLeftFront(corners[1].x, corners[1].z);

	if(middleInZeros) {
		// nekoliduje v Y souřadnici
		if(!((futurePosition.y + formulaHeight/2) >= (object.position.y - object.height/2) &&
			(object.position.y + object.height/2) >= (futurePosition.y - formulaHeight/2))) {
			return false;
		}

		GLfloat xLow = (object.position.x - object.width/2);
		GLfloat xHigh = (object.position.x + object.width/2);
		GLfloat zLow = (object.position.z - object.width/2);
		GLfloat zHigh = (object.position.z + object.width/2);

		// krajní body objektu, do které formule potenciálně naráží
		glm::vec2 objectXLowZLow(xLow, zLow);
		glm::vec2 objectXHighZLow(xHigh, zLow);
		glm::vec2 objectXLowZHigh(xLow, zHigh);
		glm::vec2 objectXHighZHigh(xHigh, zHigh);

		if (isPointInRectangle(objectXLowZLow, formulaLeftFront, formulaRightFront, formulaRightRear, formulaLeftRear) ||
			isPointInRectangle(objectXHighZLow, formulaLeftFront, formulaRightFront, formulaRightRear, formulaLeftRear) ||
			isPointInRectangle(objectXLowZHigh, formulaLeftFront, formulaRightFront, formulaRightRear, formulaLeftRear) ||
			isPointInRectangle(objectXHighZHigh, formulaLeftFront, formulaRightFront, formulaRightRear, formulaLeftRear)) {
			return true;
		} else if(isPointInRectangle(formulaLeftRear, objectXLowZLow, objectXHighZLow, objectXHighZHigh, objectXLowZHigh) ||
			isPointInRectangle(formulaLeftFront, objectXLowZLow, objectXHighZLow, objectXHighZHigh, objectXLowZHigh) ||
			isPointInRectangle(formulaRightFront, objectXLowZLow, objectXHighZLow, objectXHighZHigh, objectXLowZHigh) ||
			isPointInRectangle(formulaRightRear, objectXLowZLow, objectXHighZLow, objectXHighZHigh, objectXLowZHigh)) {
			return true;
		} else {
			return false;
		}
	} else {
		// nekoliduje v Y souřadnici
		if(!((futurePosition.y + formulaHeight/2) >= object.position.y &&
			object.position.y + object.height >= (futurePosition.y - formulaHeight/2))) {
			return false;
		}

		GLfloat xLow = object.position.x;
		GLfloat xHigh = (object.position.x + object.width);
		GLfloat zLow = object.position.z;
		GLfloat zHigh = (object.position.z + object.width);

		// krajní body objektu, do které formule potenciálně naráží
		glm::vec2 objectXLowZLow(xLow, zLow);
		glm::vec2 objectXHighZLow(xHigh, zLow);
		glm::vec2 objectXLowZHigh(xLow, zHigh);
		glm::vec2 objectXHighZHigh(xHigh, zHigh);

		if (isPointInRectangle(objectXLowZLow, corners[1], corners[2], corners[3], corners[0]) ||
			isPointInRectangle(objectXHighZLow, corners[1], corners[2], corners[3], corners[0]) ||
			isPointInRectangle(objectXLowZHigh, corners[1], corners[2], corners[3], corners[0]) ||
			isPointInRectangle(objectXHighZHigh, corners[1], corners[2], corners[3], corners[0])) {
			return true;
		} else if(isPointInRectangle(corners[0], objectXLowZLow, objectXHighZLow, objectXHighZHigh, objectXLowZHigh) ||
			isPointInRectangle(corners[1], objectXLowZLow, objectXHighZLow, objectXHighZHigh, objectXLowZHigh) ||
			isPointInRectangle(corners[2], objectXLowZLow, objectXHighZLow, objectXHighZHigh, objectXLowZHigh) ||
			isPointInRectangle(corners[3], objectXLowZLow, objectXHighZLow, objectXHighZHigh, objectXLowZHigh)) {
			return true;
		} else {
			return false;
		}
	}
}

bool Game::isPointInRectangle(const glm::vec2& point, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& d) {
	glm::vec2 ab = b - a;
	glm::vec2 bc = c - b;
	glm::vec2 cd = d - c;
	glm::vec2 da = a - d;

	glm::vec2 ap = point - a;
	glm::vec2 bp = point - b;
	glm::vec2 cp = point - c;
	glm::vec2 dp = point - d;

	float cross1 = glm::dot(glm::vec2(-ab.y, ab.x), ap);
	float cross2 = glm::dot(glm::vec2(-bc.y, bc.x), bp);
	float cross3 = glm::dot(glm::vec2(-cd.y, cd.x), cp);
	float cross4 = glm::dot(glm::vec2(-da.y, da.x), dp);

	return cross1 >= 0 && cross2 >= 0 && cross3 >= 0 && cross4 >= 0;
}


glm::vec3 Game::calculate_collisions(glm::vec3 position, glm::vec3 offset, std::unordered_map<std::string, Mesh> &scene) {
	bool middleInZeros;
	for (auto& model : scene) {
		if(model.first == "surface") {
			middleInZeros = false;
		} else {
			middleInZeros = true;
		}
		if (check_collision(position, offset, model.second, middleInZeros)) {
			return glm::vec3(0.0);
		}
	}
	return offset;
}

bool Game::check_collision(glm::vec3 position, glm::vec3 offset, Mesh object, bool middleInZeros) { // AABB - AABB collision
	glm::vec3 futurePosition = position + offset;
	GLfloat personWidth = 0.4f;
	GLfloat personHeight = 1.2f;

	// pokud se jedná o objekty, které mají střed v bodě [0, 0, 0]
	if(middleInZeros) {
		bool collisionX = (futurePosition.x + personWidth/2) >= (object.position.x - object.width/2) &&
		(object.position.x + object.width/2) >= (futurePosition.x - personWidth/2);

		bool collisionY = futurePosition.y >= (object.position.y - object.height/2) &&
			(object.position.y + object.height/2) >= futurePosition.y - personHeight;	// výšku kamery nedělím 2, protože "oči" jsou nahoře

		bool collisionZ = (futurePosition.z + personWidth/2) >= (object.position.z - object.width/2) &&
		(object.position.z + object.width/2) >= (futurePosition.z - personWidth/2);
		return collisionX && collisionY && collisionZ;

	// ostatní objekty, které mají v bodě [0, 0, 0] začátek a mají pouze plusové souřadnice
	} else {
		bool collisionX = (futurePosition.x + personWidth/2) + personWidth >= object.position.x &&
			object.position.x + object.width >= (futurePosition.x - personWidth/2);

		bool collisionY = futurePosition.y >= object.position.y &&
			object.position.y + object.height >= futurePosition.y - personHeight;	// výšku kamery nedělím 2, protože "oči" jsou nahoře

		bool collisionZ = (futurePosition.z + personWidth/2) >= object.position.z &&
			object.position.z + object.width >= (futurePosition.z - personWidth/2);
		return collisionX && collisionY && collisionZ;
	}
}

void Game::update_dynamic_models() {
	// formula
	if(driveMode) {
		set_formula_model_position();
	}

	// wheel
	wheelPosition.z = 50.f + 48.0f * sin(glfwGetTime() * 0.3f);
	glm::mat4 wheel_model = glm::scale(glm::rotate(glm::translate(glm::identity<glm::mat4>(), wheelPosition), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(0.015,0.015, 0.015));
	wheel_model = glm::rotate(wheel_model, wheelPosition.z, glm::vec3(0.0f, 0.0f, 1.0f));
	scene["wheel"].setModelMatrix(wheel_model);
	scene["wheel"].position = wheelPosition;

	// drone 1
	// Aktualizace pozice dronu + zaøídit rotaci objektu jako takového a aby to svìtlo skokovì neskákalo
	float droneSpeed = 0.05f; // Rychlost pohybu dronu
	glm::vec3 direction = drone1TargetPositions[currentTrianglePoint] - drone1Position;
	if (glm::length(direction) < droneSpeed) {
		// Pøepnutí na další bod trojúhelníku
		currentTrianglePoint = (currentTrianglePoint + 1) % 3;
		direction = drone1TargetPositions[currentTrianglePoint] - drone1Position;
	}
	drone1Position += glm::normalize(direction) * droneSpeed;
	scene["drone1"].setModelMatrix(glm::translate(glm::identity<glm::mat4>(), drone1Position));

	// drone 2
	glm::vec3 drone_position2 = scene["drone2"].position;
	drone_position2.y = 8.0f + 2.0f * sin(glfwGetTime()); // Example of up and down movement
	scene["drone2"].position = drone_position2;
	glm::mat4 drone_model2 = glm::translate(glm::identity<glm::mat4>(), drone_position2);
	//drone_model = glm::scale(drone_model, glm::vec3(1.0f, 1.0f, 1.0f)); // Scaling as necessary
	scene["drone2"].setModelMatrix(drone_model2);

	// drone 3
	glm::vec3 drone_position3 = scene["drone3"].position;
	//drone_position3.y = 8.0f + 2.0f * sin(glfwGetTime()); // Example of up and down movement
	drone_position3.x = 50.f + 25.0f * sin(glfwGetTime() * 0.3f); // Example of movement along Z axis
	scene["drone3"].position = drone_position3;
	glm::mat4 drone_model3 = glm::translate(glm::identity<glm::mat4>(), drone_position3);
	// drone_model3 = glm::scale(drone_model3, glm::vec3(1.0f, 1.0f, 1.0f)); // Scaling as necessary
	scene["drone3"].setModelMatrix(drone_model3);
}

void Game::set_formula_model_position() {
	glm::vec3 formula_position = get_formula_position_coordinates();

	// černá formule - složitější model, náročnější zobrazení
	glm::mat4 modelMatrix = glm::translate(glm::identity<glm::mat4>(), formula_position);
	modelMatrix = glm::rotate(modelMatrix, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	float angle = glm::atan(camera.Front.x, camera.Front.z);
	modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
	scene["formula"].setModelMatrix(modelMatrix);
	scene["formula"].position = formula_position;

	// červená formule - jednoduchý model
	/*glm::mat4 modelMatrix = glm::translate(glm::identity<glm::mat4>(), formula_position);
	float angle = glm::atan(camera.Front.x, camera.Front.z);	// vypočítání úhlu otáčení na základě směru kamery
	modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f, 0.25f, 0.25f));
	scene["formula"].setModelMatrix(modelMatrix);
	scene["formula"].position = formula_position;*/
}

glm::vec3 Game::get_formula_position_coordinates() {
	glm::vec3 formula_position = camera.Position + camera.Front * 4.5f;
	formula_position.y = scene["formula"].height/2;
	return formula_position;
}

void Game::set_lights() {
// Nastavení směrového světla (jaké množství je přidáno do scény)
shader->setUniform("dirLight.direction", glm::vec3(1.0f, -0.8f, 1.0f)); // Směr světla
shader->setUniform("dirLight.ambient", glm::vec3(0.3f, 0.3f, 0.3f)); // Ambientní složka
shader->setUniform("dirLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f)); // Difúzní složka
shader->setUniform("dirLight.specular", glm::vec3(0.9f, 0.9f, 0.9f)); // Specularní složka

// Jaký podíl XX světla objekty odráží
shader->setUniform("material.ambient", glm::vec3(0.6));
shader->setUniform("material.shininess", 32.0f);
shader->setUniform("material.transparency", 1.0f);
//shader->setUniform("material.transparency", scene["glass"].material.transparency);

// Nastavení bodového světla
//shader->setUniform("viewPos", camera.Position);

// Point light 1
shader->setUniform("pointLights[0].position", pointLightPositions[0]);
shader->setUniform("pointLights[0].ambient", glm::vec3(0.0f, 0.0f, 0.9f));
shader->setUniform("pointLights[0].diffuse", glm::vec3(0.0f, 0.0f, 3.0f));
shader->setUniform("pointLights[0].specular", glm::vec3(0.0f, 0.0f, 3.0f));
shader->setUniform("pointLights[0].constant", 1.0f);
shader->setUniform("pointLights[0].linear", 0.09f);
shader->setUniform("pointLights[0].quadratic", 0.032f);

// Point light 2
shader->setUniform("pointLights[1].position", pointLightPositions[1]);
shader->setUniform("pointLights[1].ambient", glm::vec3(0.9f, 0.1f, 0.0f));
shader->setUniform("pointLights[1].diffuse", glm::vec3(3.0f, 0.0f, 0.0f));
shader->setUniform("pointLights[1].specular", glm::vec3(3.0f, 0.0f, 0.0f));
shader->setUniform("pointLights[1].constant", 1.0f);
shader->setUniform("pointLights[1].linear", 0.09f);
shader->setUniform("pointLights[1].quadratic", 0.032f);

// Semafor light (Point light 3)
if (red_light_on_semafor) {
shader->setUniform("pointLights[2].position", pointLightPositions[2]);
shader->setUniform("pointLights[2].ambient", glm::vec3(0.9f, 0.0f, 0.0f));
shader->setUniform("pointLights[2].diffuse", glm::vec3(1.0f, 0.0f, 0.0f));
shader->setUniform("pointLights[2].specular", glm::vec3(1.0f, 0.0f, 0.0f));
shader->setUniform("pointLights[2].constant", 1.0f);
shader->setUniform("pointLights[2].linear", 0.09f);
shader->setUniform("pointLights[2].quadratic", 0.032f);
}
else {
shader->setUniform("pointLights[2].position", pointLightPositions[2]);
shader->setUniform("pointLights[2].ambient", glm::vec3(0.0f, 0.9f, 0.0f));
shader->setUniform("pointLights[2].diffuse", glm::vec3(0.0f, 1.0f, 0.0f));
shader->setUniform("pointLights[2].specular", glm::vec3(0.0f, 1.0f, 0.0f));
shader->setUniform("pointLights[2].constant", 1.0f);
shader->setUniform("pointLights[2].linear", 0.09f);
shader->setUniform("pointLights[2].quadratic", 0.032f);
}

//Spotlight pro 3 dron
// Synchronizace spotlight s pohyben drona1
glm::vec3 drone_position3 = scene["drone3"].position;
glm::vec3 spotlightPosition = drone_position3;

// Nstavení světla tak, aby svítilo od dronu na zem a 6 jednotek před sebe
glm::vec3 targetPosition = drone_position3 + glm::vec3(6.0f, 0.0f, 0.0f);
targetPosition.y = 1.0f;

// Calculate spotlight direction from spotlightPosition to targetPosition
glm::vec3 spotlightDirection = glm::normalize(targetPosition - spotlightPosition);

shader->setUniform("spotLight.position", spotlightPosition);
shader->setUniform("spotLight.direction", spotlightDirection);
shader->setUniform("spotLight.ambient", glm::vec3(5.0f, 5.0f, 5.0f));
shader->setUniform("spotLight.diffuse", glm::vec3(3.0f, 3.0f, 3.0f));
shader->setUniform("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
shader->setUniform("spotLight.constant", 1.0f);
shader->setUniform("spotLight.linear", 0.09f);
shader->setUniform("spotLight.quadratic", 0.032f);
shader->setUniform("spotLight.cutOff", glm::cos(glm::radians(10.5f)));
shader->setUniform("spotLight.outerCutOff", glm::cos(glm::radians(13.0f)));


shader->setUniform("uVm", camera.GetViewMatrix());
}

void Game::draw_all_models() {
	// Povolení blendingu
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (auto& model : scene) {
		if (model.first == "glass") {
			shader->setUniform("material.transparency", 0.4f); // Nastavení průhlednosti pro objekt glass
		}
		else {
			shader->setUniform("material.transparency", 1.0f); // Výchozí průhlednost pro ostatní objekty
		}
		model.second.draw();
	}
	// Zakázání blendingu
	glDisable(GL_BLEND);
}

void Game::handle_semafor_change(synced_deque<bool>& queue, bool red_tracker_on) {
	// check if red object in front of screen
	if(red_tracker_on && !queue.empty()) {
		red_light_on_semafor = queue.pop_front();
	}
}

void Game::update_projection_matrix(void) {
	if (height < 1)
		height = 1;   // avoid division by 0

	float ratio = static_cast<float>(width) / height;
	projection_matrix = glm::perspective(
		glm::radians(fov), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
		ratio,               // Aspect Ratio. Depends on the size of your window.
		0.01f,                // Near clipping plane. Keep as big as possible, or you'll get precision issues.
		20000.0f              // Far clipping plane. Keep as little as possible.
	);
}
