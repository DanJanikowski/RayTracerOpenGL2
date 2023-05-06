// debug_malloc.cpp
// compile by using: cl /EHsc /W4 /D_DEBUG /MDd debug_malloc.cpp
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "globals.h"
#include "scene.h"


// Sets up the opengl window, creates Environment variable and calls its draw function
int main() {
	// Init GLFW
	glfwInit();
	// Tell GLFW we are using version 4.6
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(globals::WINDOW_WIDTH, globals::WINDOW_HEIGHT, "Test", NULL, NULL);
	glfwMakeContextCurrent(window); // contexts are weird, basically makes the window viewable

	glfwSetWindowPos(window, 400, 50);
	

	// Basically load up OpenGL
	gladLoadGL();
	// Setup the viewport
	glViewport(0, 0, globals::WINDOW_WIDTH, globals::WINDOW_HEIGHT);
	//glEnable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);


	Scene* scene = new Scene(window);
	scene->draw();
	delete scene;


	glfwDestroyWindow(window);
	glfwTerminate();

	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtDumpMemoryLeaks();
}