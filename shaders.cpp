#include "shaders.h"

std::string getFileContents(const char* filename) {
	std::ifstream in(filename, std::ios::binary);
	if (in) {
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}

Shader::Shader(const char* vertexFile, const char* fragmentFile, const char* computeFile) {

	std::string vertexCode = getFileContents(vertexFile);
	std::string fragmentCode = getFileContents(fragmentFile);

	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	// Setup the shaders so we can actually see objects
	// Create the vertex shader (manages the vertices)
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	// Create the fragment shader (manages pixels between the vertices)
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	screenQuadShaderID = glCreateProgram();
	glAttachShader(screenQuadShaderID, vertexShader);
	glAttachShader(screenQuadShaderID, fragmentShader);
	glLinkProgram(screenQuadShaderID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);



	// Setup compute shader program
	std::string computeCode = getFileContents(computeFile);
	const char* computeSource = computeCode.c_str();
	// Setup the shaders so we can actually see objects
	// Create the vertex shader (manages the vertices)
	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeShader, 1, &computeSource, NULL);
	glCompileShader(computeShader);

	compShaderID = glCreateProgram();
	glAttachShader(compShaderID, computeShader);
	glLinkProgram(compShaderID);

	glDeleteShader(computeShader);
}

void Shader::activateDefaultShader() {
	glUseProgram(screenQuadShaderID);
}

void Shader::activateCompShader() {
	glUseProgram(compShaderID);
}

void Shader::deleteShaders() {
	glDeleteProgram(screenQuadShaderID);
	glDeleteProgram(compShaderID);
}