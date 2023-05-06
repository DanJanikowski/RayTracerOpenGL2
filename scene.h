#pragma once

#include <string>
#include <vector>

#include <omp.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "globals.h"
#include "shaders.h"
#include "camera.h"
#include "object.h"

class Scene {
public:
	Scene(GLFWwindow* window_);
	~Scene();

	void keyInput(int key, int scancode, int action, int mods);

	void draw();

	GLFWwindow* window;
	Camera* camera;
	Shader* shaders;

private:
	static void keyInputSetup(GLFWwindow* window, int key, int scancode, int action, int mods) {
		Scene* tempEnv = static_cast<Scene*>(glfwGetWindowUserPointer(window));
		tempEnv->keyInput(key, scancode, action, mods);
	}

	int randmode = 0;
	bool resetFrames = false;

	void updateFPS();
	void setupScreenQuad();
	void setupSceneObjects();
	void setupComputeShaderData();

	const unsigned int COMP_DIM_X, COMP_DIM_Y;

	std::vector<Sphere> spheresVec;
	std::vector<Quad> quadsVec;
	std::vector<PointLight> pointLightsVec;

	// Variables for textured screen quad
	GLuint texID;
	GLuint VBO, EBO, VAO;
	const unsigned int TEXTURE_WIDTH, TEXTURE_HEIGHT;

	std::vector<GLfloat> screenQuadVerts;
	std::vector<GLuint> screenQuadInds;

	glm::vec4 frust00 = glm::vec4(-1, -1, 1, 1);
	glm::vec4 frust10 = glm::vec4(1, -1, 1, 1);
	glm::vec4 frust01 = glm::vec4(-1, 1, 1, 1);
	glm::vec4 frust11 = glm::vec4(1, 1, 1, 1);
	glm::vec3 ray00;
	glm::vec3 ray10;
	glm::vec3 ray01;
	glm::vec3 ray11;

	// FPS variables
	double prevTime = 0.0;
	double curTime = 0.0;
	unsigned int counter = 0;
	float frameTime = 0.0001;

	// Uniform locations
	GLuint skyboxID;
	GLuint timeLoc, cameraPosLoc, cameraDirLoc, numAccumFramesLoc, ray00Loc, ray10Loc, ray01Loc, ray11Loc;
	GLuint textureLoc;
};