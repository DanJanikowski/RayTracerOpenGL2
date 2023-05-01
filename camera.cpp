
#include "Camera.h"

Camera::Camera(GLFWwindow* windowptr) {
	position = glm::vec3(0.0f, 0.0f, 2.0f);
	window = windowptr;
	matrix();
}
Camera::~Camera() {
	window = nullptr;
}

void Camera::matrix() {
	// Initializes matrices since otherwise they will be the null matrix
	view = glm::mat4(1.0f);
	projection = glm::mat4(1.0f);

	// Makes camera look in the right direction from the right position
	view = glm::lookAt(position, position + direction, Up);
	// Adds perspective to the scene
	projection = glm::perspective(glm::radians(FOV), (float)globals::WINDOW_WIDTH / globals::WINDOW_HEIGHT, NEAR_PLANE, FAR_PLANE);

	invProjView = glm::inverse(projection * view);
}


void Camera::inputs(const float& frameTime, bool& resetFrames) {
	// Handles key inputs

	float curVel = slowVel;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		curVel = fastVel;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position += curVel * frameTime * direction;
		resetFrames = true;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position += curVel * frameTime * -glm::normalize(glm::cross(direction, Up));
		resetFrames = true;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position += curVel * frameTime * -direction;
		resetFrames = true;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += curVel * frameTime * glm::normalize(glm::cross(direction, Up));
		resetFrames = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		position += curVel * frameTime * Up;
		resetFrames = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		position += curVel * frameTime * -Up;
		resetFrames = true;
	}


	// Handles mouse inputs (camera movement)
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		resetFrames = true;
		// Hides mouse cursor
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		// Prevents camera from jumping on the first click
		if (firstClick) {
			glfwSetCursorPos(window, (globals::WINDOW_WIDTH / 2), (globals::WINDOW_HEIGHT / 2));
			firstClick = false;
		}

		// Stores the coordinates of the cursor
		double mouseX;
		double mouseY;
		// Fetches the coordinates of the cursor
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// Normalizes and shifts the coordinates of the cursor such that they begin in the middle of the screen
		// and then "transforms" them into degrees 
		float rotX = sensitivity * (float)(mouseY - (globals::HALF_WH)) / globals::WINDOW_HEIGHT;
		float rotY = sensitivity * (float)(mouseX - (globals::HALF_WW)) / globals::WINDOW_WIDTH;

		// Calculates upcoming vertical change in the Orientation
		glm::vec3 newOrientation = glm::rotate(direction, glm::radians(-rotX), glm::normalize(glm::cross(direction, Up)));

		// Decides whether or not the next vertical Orientation is legal or not
		if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f)) {
			direction = newOrientation;
		}

		// Rotates the Orientation left and right
		direction = glm::rotate(direction, glm::radians(-rotY), Up);

		// Sets mouse cursor to the middle of the screen so that it doesn't end up roaming around
		glfwSetCursorPos(window, (globals::HALF_WW), (globals::HALF_WH));
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		// Unhides cursor since camera is not looking around anymore
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		// Makes sure the next time the camera looks around it doesn't jump
		firstClick = true;
	}
}