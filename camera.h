#pragma once

#define GLM_FORCE_RADIANS

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/string_cast.hpp>

#include "globals.h"

class Camera {
public:
	// Camera constructor to set up initial values
	Camera(GLFWwindow* windowptr);
	~Camera();

	// Updates and exports the camera matrix to the Vertex Shader
	void matrix();

	// Handles camera inputs
	void inputs(const float& frameTime, bool& resetFrames);

	GLFWwindow* window;

	// Prevents the camera from jumping around when first clicking left click
	bool firstClick = true;

	// Adjust the speed of the camera and it's sensitivity when looking around
	const float slowVel = 4.0f;
	const float fastVel = 8.0f;
	const float sensitivity = 100.0f;

	// Camera parameters
	const float FOV = 45.0f, NEAR_PLANE = 0.01f, FAR_PLANE = 1000.0f;

	// Stores the main vectors of the camera
	glm::vec3 position;
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f);
	const glm::vec3 Up = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 invProjView;
};