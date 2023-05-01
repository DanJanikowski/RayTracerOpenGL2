#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>

std::string getFileContents(const char* filename);

class Shader {
public:
	GLuint screenQuadShaderID;
	GLuint compShaderID;
	Shader(const char* vertexFile, const char* fragmentFile, const char* computeFile);

	void activateDefaultShader();
	void activateCompShader();

	void deleteShaders();
};