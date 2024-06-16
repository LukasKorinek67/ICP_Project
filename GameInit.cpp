#include "Game.h"

#include <iostream>
#include <filesystem>
#include <vector>
#include <opencv2/opencv.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Texture.h"
#include "Map.h"

Game::Game() {
	std::cout << "Constructed...\n";
}

Game::~Game() {
	// clean-up
	cv::destroyAllWindows();
	// clean-up GLFW
	glfwTerminate();

	//new stuff: cleanup GL data
	//glDeleteProgram(shader_prog_ID);
	//glDeleteBuffers(1, &VBO_ID);
	//glDeleteVertexArrays(1, &VAO_ID);
}

bool Game::init() {

	//if (!std::filesystem::exists("../resources")) {
	if (!std::filesystem::exists("resources")) {
		//throw std::exception("Directory 'resources' not found. Various media files are expected to be there.");
		throw std::runtime_error("Directory 'resources' not found. Various media files are expected to be there.");
	}

	init_glfw();
	init_glew();
	init_gl_debug();
	print_startup_info();

	glfwSwapInterval(swap_interval); // vsync
	glClearColor(0.0, 0.7, 1.3, 0.0);

	set_callbacks();

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glCullFace(GL_BACK);		// Urèuje, které strany (faces) polygonù by mìly být odstranìny (cullovány) - zadní strany (které nejsou viditelné) nebudou vykreslovány - zvýšení výkonu
	glEnable(GL_CULL_FACE);		// Povolení odstranìní zadních stran (face culling). OpenGL nebude vykreslovat zadní strany polygonù - zvýšení výkonu
	glDepthFunc(GL_LEQUAL);			// Nastavení funkce pro testování hloubky - GL_LEQUAL znamená, že pixel projde testem hloubky, pokud jeho hloubka je menší nebo rovna hodnotì uložené v hloubkovém bufferu
	glEnable(GL_DEPTH_TEST);	// Povolení testování hloubky - OpenGL používá hloubkový buffer pro urèení, které objekty jsou pøed nebo za jinými objekty
	glEnable(GL_LINE_SMOOTH);	// Povolení vyhlazování èar - budou vypadat ménì zubatì
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// Nastavuje funkci pro míchání (blending) barev. Pokud má objekt alfa hodnotu 0.5, bude tento objekt poloprùhledný

	init_assets();
	glViewport(0, 0, width, height);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetCursorPos(window, &xpos_prev, &ypos_prev);
	update_projection_matrix();
	shader->setUniform("uPm", projection_matrix);
	driveMode = true;
	return true;
}

void Game::init_assets() {
	//std::filesystem::path cp = std::filesystem::current_path().parent_path() / "resources";
	std::filesystem::path cp = std::filesystem::current_path().parent_path() / "ICP_Game/resources";
	//std::filesystem::path cp = std::filesystem::current_path() / ".." / "resources";
	std::filesystem::path vsPath = (cp / "shaders" / "obj.vert");
	std::filesystem::path fsPath = (cp / "shaders" / "obj.frag");

	ShaderProgram* sh = new ShaderProgram(vsPath, fsPath);
	this->shader = sh;
	shader->activate();
	update_projection_matrix();

	// Map
	Map map = Map((cp / "map" / "map.png").u8string());
	if (map.load()) {
		std::cout << "Map loaded\n";
	}
	else {
		std::cerr << "Failed to load map\n";
	}
	camera.Position = { map.start_position.first, 1.5, map.start_position.second };

	// plocha
	GLuint texture_surface = textureInit((cp / "textures" / "race_track.png").u8string(), false);
	glm::vec3 surface_position = glm::vec3(0.0f, 0.0f, 0.0f);
	scene["surface"] = Mesh((cp / "models" / "surface.obj").u8string(), *sh, texture_surface);
	scene["surface"].height = 0.01f;
	scene["surface"].width = 100.0f;
	scene["surface"].position = surface_position;
	scene["surface"].setModelMatrix(glm::scale(glm::translate(glm::identity<glm::mat4>(), surface_position), glm::vec3(100, 1.0, 100)));

	// kužely a ohranièení mapy
	GLuint texture_border = textureInit((cp / "textures" / "terrazzo.png").u8string(), false);
	GLuint texture_cone = textureInit((cp / "textures" / "cone.jpg").u8string(), false);
	Mesh cube = Mesh((cp / "models" / "cube.obj").u8string(), *sh, texture_border);
	Mesh cone = Mesh((cp / "models" / "cone.obj").u8string(), *sh, texture_cone);
	glm::vec3 border_position;
	glm::vec3 cone_position;

	for (int i = 0; i < map.positions.size(); i++) {
		int r, g, b;
		std::tie(r, g, b) = map.colors[i];

		if (r == 0 && g == 0 && b == 255) {
			border_position = glm::vec3((float)map.positions[i].first, 0.5f, (float)map.positions[i].second);
			glm::mat4 modelMatrix = glm::scale(glm::translate(glm::identity<glm::mat4>(), border_position), glm::vec3(1.0, 1.0, 1.0));
			cube.setModelMatrix(modelMatrix);
			//cube.position = { map.positions[i].first + 0.5f,0.1f,(float) map.positions[i].second + 0.5f};
			cube.position = border_position;
			cube.width = 1.0f;
			cube.height = 1.0f;
			scene["cube" + std::to_string(i)] = cube;
			continue;
		}
		if (r == 255 && g == 140 && b == 0) {
			cone_position = glm::vec3((float)map.positions[i].first, 0.4f, (float)map.positions[i].second);
			glm::mat4 modelMatrix = glm::scale(glm::translate(glm::identity<glm::mat4>(), cone_position), glm::vec3(0.02, 0.02, 0.02));
			cone.setModelMatrix(modelMatrix);
			//cone.position = { map.positions[i].first + 0.5f,0.1f,(float) map.positions[i].second + 0.5f};
			cone.position = cone_position;
			cone.width = 0.5f;
			cone.height = 0.8f;
			scene["cone" + std::to_string(i)] = cone;
			continue;
		}
	}

	// èerná formule - složitìjší model, nároènìjší zobrazení
	// délka 5,1f, šíøka 1,95f, výška 1,2f
	GLuint texture_formula = textureInit((cp / "textures" / "formula_black.png").u8string(), false);
	scene["formula"] = Mesh((cp / "models" / "formula_new.obj").u8string(), *sh, texture_formula);
	scene["formula"].width = 5.1f;
	scene["formula"].height = 1.2f;
	set_formula_model_position();

	// èervená formule - jednoduchý model
	// délka 4,2f, šíøka 1,65f, výška 1,05f
	/*GLuint texture_formula = textureInit((cp / "textures" / "formula_red.jpg").u8string(), false);
	scene["formula"] = Mesh((cp / "models" / "formula_old.obj").u8string(), *sh, texture_formula);
	scene["formula"].width = 4.2f;
	scene["formula"].height = 1.05f;
	set_formula_model_position();*/

	// kolo - šíøka 0.7f, délka 0.3f
	GLuint texture_wheel = textureInit((cp / "textures" / "wheel.png").u8string(), false);
	wheelPosition = glm::vec3(32.0f, 0.6f, 10.0f);
	scene["wheel"] = Mesh((cp / "models" / "wheel.obj").u8string(), *sh, texture_wheel);
	scene["wheel"].position = wheelPosition;
	scene["wheel"].width = 0.7f;
	scene["wheel"].height = 0.9f;
	scene["wheel"].setModelMatrix(glm::scale(glm::rotate(glm::translate(glm::identity<glm::mat4>(), wheelPosition), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(0.015, 0.015, 0.015)));

	// semafor
	GLuint texture_traffic_signal = textureInit((cp / "textures" / "traffic_signal.jpg").u8string(), false);
	glm::vec3 traffic_signal_position = glm::vec3(87.6f, 3.5f, 40.0f);
	scene["traffic-signal"] = Mesh((cp / "models" / "traffic_signal.obj").u8string(), *sh, texture_traffic_signal);
	scene["traffic-signal"].position = traffic_signal_position;
	scene["traffic-signal"].width = 1.5f;
	scene["traffic-signal"].height = 2.0f;
	scene["traffic-signal"].setModelMatrix(glm::scale(glm::rotate(glm::rotate(glm::translate(glm::identity<glm::mat4>(), traffic_signal_position), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, -1.0f)), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(0.2f, 0.2f, 0.2f)));

	// drony
	GLuint texture_drone = textureInit((cp / "textures" / "drone.png").u8string(), false);

	// definice bodù pro pohyb dronu ve vodorovné rovinì
	drone1TargetPositions[0] = glm::vec3(10.0f, 5.0f, 10.0f);
	drone1TargetPositions[1] = glm::vec3(40.0f, 5.0f, 10.0f);
	drone1TargetPositions[2] = glm::vec3(25.0f, 5.0f, 40.0f);

	drone1Position = drone1TargetPositions[0];
	currentTrianglePoint = 0;
	droneSpeed = 0.05f;

	scene["drone1"] = Mesh((cp / "models" / "drone.obj").u8string(), *sh, texture_drone);
	scene["drone1"].position = drone1Position;
	scene["drone1"].width = 1.0f;
	scene["drone1"].height = 1.0f;
	scene["drone1"].setModelMatrix(glm::translate(glm::identity<glm::mat4>(), drone1Position));

	glm::vec3 drone2_position = glm::vec3(80.0f, 6.0f, 35.0f);
	scene["drone2"] = Mesh((cp / "models" / "drone.obj").u8string(), *sh, texture_drone);
	scene["drone2"].position = drone2_position;
	scene["drone2"].width = 1.0f;
	scene["drone2"].height = 1.0f;
	scene["drone2"].setModelMatrix(glm::scale(glm::translate(glm::identity<glm::mat4>(), drone2_position), glm::vec3(1.0f, 1.0f, 1.0f)));

	glm::vec3 drone3_position = glm::vec3(80.0f, 6.0f, 87.0f);
	scene["drone3"] = Mesh((cp / "models" / "drone.obj").u8string(), *sh, texture_drone);
	scene["drone3"].position = drone3_position;
	scene["drone3"].width = 1.0f;
	scene["drone3"].height = 1.0f;
	scene["drone3"].setModelMatrix(glm::scale(glm::translate(glm::identity<glm::mat4>(), drone3_position), glm::vec3(1.0f, 1.0f, 1.0f)));

	// Glass - transparence
	GLuint texture_glass = textureInit((cp / "textures" / "glass.jpg").u8string(), false);
	glm::vec3 glass_position = glm::vec3(80.0f, 0.2f, 35.0f);
	scene["glass"] = Mesh((cp / "models" / "glass.obj").u8string(), *sh, texture_glass);
	scene["glass"].position = glass_position;
	scene["glass"].width = 0.5f;
	scene["glass"].height = 0.8f;
	scene["glass"].setModelMatrix(glm::scale(glm::translate(glm::identity<glm::mat4>(), glass_position), glm::vec3(7.0f, 7.0f, 7.0f)));
}

void Game::init_glfw(void) {
	if (!glfwInit()) {
		//throw std::exception("GLFW can not be initialized.");
		throw std::runtime_error("GLFW can not be initialized.");
	}

	// try to open OpenGL 4.3 - TODO: verze 4.3 na Macu nefunguje, "nejnovìjší" je 4.1
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// only on Mac
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(1280, 800, "ICP Project - Korinek & Doanova", NULL, NULL);

	if (!window) {
		glfwTerminate();
		//throw std::exception("GLFW window can not be created.");
		throw std::runtime_error("GLFW window can not be created.");
	}
	// Make the window's context current
	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &width, &height); // tohle mi do width a height uloží velikost okna
	glfwSetWindowUserPointer(window, this);
}

void Game::init_glew(void) {
	//
	// Initialize all valid generic GL extensions with GLEW.
	// Usable AFTER creating GL context! (create with glfwInit(), glfwCreateWindow(), glfwMakeContextCurrent()
	//
	{
		GLenum glew_ret;
		glew_ret = glewInit();
		if (glew_ret != GLEW_OK) {
			//throw std::exception(std::string("GLEW failed with error: ").append((const char*)glewGetErrorString(glew_ret)).append("\n").c_str());
			throw std::runtime_error(std::string("GLEW failed with error: ").append((const char*)glewGetErrorString(glew_ret)).append("\n").c_str());
		}
		else {
			std::cout << "GLEW successfully initialized to version: " << glewGetString(GLEW_VERSION) << "\n";
		}

		// Platform specific init. (Change to GLXEW or ELGEW if necessary.)
		/*glew_ret = wglewInit();
		glew_ret = glewInit();
		if (glew_ret != GLEW_OK) {
			//throw std::exception(std::string("WGLEW failed with error: ").append((const char*)glewGetErrorString(glew_ret)).append("\n").c_str());
			throw std::runtime_error((std::string("WGLEW failed with error: ").append((const char*)glewGetErrorString(glew_ret)).append("\n").c_str()));
		}
		else {
			std::cout << "WGLEW successfully initialized platform specific functions.\n";
		}*/
	}
	/*
	{ // get extension list - výpis dostupných extensions
		GLint n = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &n);
		for (GLint i = 0; i < n; i++) {
			const char* extension_name = (const char*)glGetStringi(GL_EXTENSIONS, i);
			std::cout << extension_name << '\n';
		}
	}
	*/
}

void Game::init_gl_debug() {
	if (GLEW_ARB_debug_output) {
		glDebugMessageCallback(MessageCallback, 0);
		glEnable(GL_DEBUG_OUTPUT);
		std::cout << "GL_DEBUG enabled." << std::endl;
	}
	else {
		std::cout << "GL_DEBUG NOT SUPPORTED!" << std::endl;
	}
}

void Game::set_callbacks() {
	glfwSetErrorCallback(error_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
}

void Game::print_startup_info() {
	std::cout << "-------------------------------------------------------" << '\n';
	std::cout << "INFO: " << '\n';
	std::cout << "-----" << '\n';
	print_glfw_info();
	print_gl_info();
	print_glm_info();
	std::cout << "-------------------------------------------------------" << '\n';
}

void Game::print_glfw_info(void) {
	int major, minor, revision;
	glfwGetVersion(&major, &minor, &revision);
	std::cout << "Running GLFW " << major << '.' << minor << '.' << revision << '\n';
	std::cout << "Compiled against GLFW "
		<< GLFW_VERSION_MAJOR << '.' << GLFW_VERSION_MINOR << '.' << GLFW_VERSION_REVISION
		<< '\n';
}

void Game::print_glm_info() {
	// GLM library
	std::cout << "GLM version: " << GLM_VERSION_MAJOR << '.' << GLM_VERSION_MINOR << '.' << GLM_VERSION_PATCH << "rev" << GLM_VERSION_REVISION << std::endl;
}

void Game::print_gl_info() {
	// get OpenGL info
	auto vendor_s = (const char*)glGetString(GL_VENDOR);
	std::cout << "OpenGL driver vendor: " << (vendor_s ? vendor_s : "UNKNOWN") << '\n';

	auto renderer_s = (const char*)glGetString(GL_RENDERER);
	std::cout << "OpenGL renderer: " << (renderer_s ? renderer_s : "<UNKNOWN>") << '\n';

	auto version_s = (const char*)glGetString(GL_VERSION);
	std::cout << "OpenGL version: " << (version_s ? version_s : "<UNKNOWN>") << '\n';

	auto glsl_s = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	std::cout << "Primary GLSL shading language version: " << (glsl_s ? glsl_s : "<UNKNOWN>") << std::endl;

	// get GL profile info
	{
		GLint profile_flags;
		glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile_flags);
		std::cout << "Current profile: ";
		if (profile_flags & GL_CONTEXT_CORE_PROFILE_BIT)
			std::cout << "CORE";
		else
			std::cout << "COMPATIBILITY";
		std::cout << std::endl;
	}

	// get context flags
	{
		GLint context_flags;
		glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
		std::cout << "Active context flags: ";
		if (context_flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
			std::cout << "GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT ";
		if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT)
			std::cout << "GL_CONTEXT_FLAG_DEBUG_BIT ";
		if (context_flags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT)
			std::cout << "GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT ";
		if (context_flags & GL_CONTEXT_FLAG_NO_ERROR_BIT)
			std::cout << "GL_CONTEXT_FLAG_NO_ERROR_BIT";
		std::cout << std::endl;
	}
}