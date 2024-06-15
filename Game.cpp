#include "Game.h"

#include <iostream>
#include <filesystem>
#include <vector>
#include <opencv2/opencv.hpp>

#include <glm/gtx/quaternion.hpp>


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
	//std::cout << offset.x << " " << offset.x <<" " << offset.z << "\n";
	if(driveMode) {
		camera.Position += calculate_formula_collisions(camera.Position, offset, scene);
	} else {
		camera.Position += calculate_collisions(camera.Position, offset, scene);
	}
	last_frame_time = nowGLFW;
	//std::cout << "X: " << camera.Position.x << ", Z: " << camera.Position.z << "\n";
}

glm::vec3 Game::calculate_formula_collisions(glm::vec3 position, glm::vec3 offset, std::unordered_map<std::string, Mesh> &scene) {
	for (auto& model : scene) {
		if (model.first != "formula" && check_collision_with_formula(position, offset, model.second)) {
			return glm::vec3(0.0);
		}
	}
	return offset;
}

bool Game::check_collision_with_formula(glm::vec3 position, glm::vec3 offset, Mesh object) { // AABB - AABB collision
	//std::cout << "position x: " << position.x << ", position y: " << position.y << ", position z: " << position.z << "\n";
	position = get_formula_position_coordinates();
	//position.y = 1.5f;

	bool collisionX = position.x + 1.1f + offset.x >= object.position.x &&
		object.position.x + object.width >= position.x + offset.x;

	/*bool collisionY = position.y + offset.y >= object.position.y &&
		object.position.y + object.height >= position.y - 1.45f + offset.y;*/
	bool collisionY = 1.5f + offset.y >= object.position.y &&
		object.position.y + object.height >= 1.5f - 1.45f + offset.y;

	bool collisionZ = position.z + 1.1f + offset.z >= object.position.z &&
		object.position.z + object.width >= position.z + offset.z;
	return collisionX && collisionY && collisionZ;
}

glm::vec3 Game::calculate_collisions(glm::vec3 position, glm::vec3 offset, std::unordered_map<std::string, Mesh> &scene) {
	for (auto& model : scene) {
		if (check_collision(position, offset, model.second)) {
			return glm::vec3(0.0);
		}
	}
	return offset;
}

bool Game::check_collision(glm::vec3 position, glm::vec3 offset, Mesh object) { // AABB - AABB collision
	bool collisionX = position.x + 1.1f + offset.x >= object.position.x &&
		object.position.x + object.width >= position.x + offset.x;

	bool collisionY = position.y + offset.y >= object.position.y &&
		object.position.y + object.height >= position.y - 1.45f + offset.y;

	bool collisionZ = position.z + 1.1f + offset.z >= object.position.z &&
		object.position.z + object.width >= position.z + offset.z;
	return collisionX && collisionY && collisionZ;
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
	glm::vec3 f1_position = camera.Position + camera.Front * 4.5f;
	f1_position.y = 0.0f;
	return f1_position;
}

void Game::set_lights() {
	// Nastavení směrového světla (jaké množstí je přidáno do scény)
	shader->setUniform("dirLight.direction", glm::vec3(1.0f, -0.8f, 1.0f)); // Smìr svìtla - toto ještì upravit
	shader->setUniform("dirLight.ambient", glm::vec3(0.3f, 0.3f, 0.3f)); // Ambientní složka
	shader->setUniform("dirLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f)); // Difúzní složka
	shader->setUniform("dirLight.specular", glm::vec3(0.9f, 0.9f, 0.9f)); // Specularní složka

	// Jaký podíl XX světla objekty odráží
	shader->setUniform("material.ambient", glm::vec3(0.6));
	//shader->setUniform("material.diffuse", glm::vec3(0.8));	//když to nastavíme a ve fragmentu násobíme difùzní material, tak je plocha strašnì tmava
	//shader->setUniform("material.specular", glm::vec3(1.0));	//také plocha ztmavne
	shader->setUniform("material.shininess", 32.0f);
	shader->setUniform("material.transparency", 1.0f);

	// Nastavení bodového světla
	//shader->setUniform("viewPos", camera.Position);

	// Point light 1
    shader->setUniform("pointLights[0].position", pointLightPositions[0]);
    shader->setUniform("pointLights[0].ambient", glm::vec3(0.0f, 0.0f, 0.9f)); // Slight blue tint for ambient light
    shader->setUniform("pointLights[0].diffuse", glm::vec3(0.0f, 0.0f, 3.0f)); // Blue diffuse light
    shader->setUniform("pointLights[0].specular", glm::vec3(0.0f, 0.0f, 3.0f)); // Blue specular light
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

	// Semafor light
	if(red_light_on_semafor) {
		shader->setUniform("pointLights[2].position", semaforLightPosition);
		shader->setUniform("pointLights[2].ambient", glm::vec3(2.0f, 0.0f, 0.0f));
		shader->setUniform("pointLights[2].diffuse", glm::vec3(3.0f, 0.0f, 0.0f));
		shader->setUniform("pointLights[2].specular", glm::vec3(3.0f, 0.0f, 0.0f));
		shader->setUniform("pointLights[2].constant", 1.0f);
		shader->setUniform("pointLights[2].linear", 0.09f);
		shader->setUniform("pointLights[2].quadratic", 0.032f);
	} else {
		shader->setUniform("pointLights[2].position", semaforLightPosition);
		shader->setUniform("pointLights[2].ambient", glm::vec3(0.0f, 2.0f, 0.0f));
		shader->setUniform("pointLights[2].diffuse", glm::vec3(0.0f, 3.0f, 0.0f));
		shader->setUniform("pointLights[2].specular", glm::vec3(0.0f, 3.0f, 0.0f));
		shader->setUniform("pointLights[2].constant", 1.0f);
		shader->setUniform("pointLights[2].linear", 0.09f);
		shader->setUniform("pointLights[2].quadratic", 0.032f);
	}

	//// Synchronize spotlight with the drone, but ensure it's at height 2.0f
	//glm::vec3 spotlightPosition = dronePosition;

	//// Calculate target position 3 units in front of the drone at ground level
	//glm::vec3 targetPosition = dronePosition + 5.0f * glm::normalize(droneTargetPositions[currentTrianglePoint] - dronePosition);
	//targetPosition.y = 2.0f; // Ground level

	//// Calculate spotlight direction from spotlightPosition to targetPosition
	//glm::vec3 spotlightDirection = glm::normalize(targetPosition - spotlightPosition);

	//shader->setUniform("spotLight.position", spotlightPosition);
	//shader->setUniform("spotLight.direction", spotlightDirection);
	//shader->setUniform("spotLight.ambient", glm::vec3(5.0f, 5.0f, 5.0f));
	//shader->setUniform("spotLight.diffuse", glm::vec3(3.0f, 3.0f, 3.0f));
	//shader->setUniform("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
	//shader->setUniform("spotLight.constant", 1.0f);
	//shader->setUniform("spotLight.linear", 0.09f);
	//shader->setUniform("spotLight.quadratic", 0.032f);
	//shader->setUniform("spotLight.cutOff", glm::cos(glm::radians(10.5f)));
	//shader->setUniform("spotLight.outerCutOff", glm::cos(glm::radians(13.0f)));

	//Spotlight pto 3 dron
	// Synchronize spotlight with the drone3
	glm::vec3 drone_position3 = scene["drone3"].position;

	// Calculate spotlight position and direction
	glm::vec3 spotlightPosition = drone_position3;

	// Calculate target position 3 units ahead on the x-axis at the same height
	glm::vec3 targetPosition = drone_position3 + glm::vec3(6.0f, 0.0f, 0.0f);
	targetPosition.y = 2.0f; // Ground level

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
	for (auto& model : scene) {
		model.second.draw();
	}
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
