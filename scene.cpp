
#include "scene.h"

Scene::Scene(GLFWwindow* window_) : TEXTURE_WIDTH(512), TEXTURE_HEIGHT(512), COMP_DIM_X(128), COMP_DIM_Y(128) {
	window = window_;
	camera = new Camera(window);
	shaders = new Shader("default.vert", "default.frag", "raytracer.comp");

	timeLoc = glGetUniformLocation(shaders->compShaderID, "time");
	cameraPosLoc = glGetUniformLocation(shaders->compShaderID, "cameraPos");
	cameraDirLoc = glGetUniformLocation(shaders->compShaderID, "cameraDir");
	numAccumFramesLoc = glGetUniformLocation(shaders->compShaderID, "numAccumFrames");
	ray00Loc = glGetUniformLocation(shaders->compShaderID, "ray00");
	ray10Loc = glGetUniformLocation(shaders->compShaderID, "ray10");
	ray01Loc = glGetUniformLocation(shaders->compShaderID, "ray01");
	ray11Loc = glGetUniformLocation(shaders->compShaderID, "ray11");
	textureLoc = glGetUniformLocation(shaders->screenQuadShaderID, "tex");

	setupScreenQuad();

	// Populate scene objects
	setupSceneObjects();
	setupComputeShaderData();
}

Scene::~Scene() {

	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteTextures(1, &texID);

	shaders->deleteShaders();

	window = nullptr;
	delete camera;
	delete shaders;
}

void Scene::setupScreenQuad() {

	// Shader and screen texture drawing setup
	glGenTextures(1, &texID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindImageTexture(0, texID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);


	GLfloat tempVerts[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	GLuint tempInds[] = {
		0, 2, 1,
		0, 3, 2
	};

	screenQuadVerts.assign(&tempVerts[0], &tempVerts[0] + 20);
	screenQuadInds.assign(&tempInds[0], &tempInds[0] + 6);


	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// Links the VBO to the VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Specifies the layout of the VBO to the VAO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBufferData(GL_ARRAY_BUFFER, screenQuadVerts.size() * sizeof(GLfloat), screenQuadVerts.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);	// Binds an element array buffer to the vertex array
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, screenQuadInds.size() * sizeof(GLuint), screenQuadInds.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/*
* Create and add all scene objects to local vectors
*/
void Scene::setupSceneObjects() {
	Material lightMaterial(glm::vec4(0, 0, 0, 300), glm::vec4(0), glm::vec4(0), glm::vec4(1));
	pointLightsVec.push_back(PointLight(glm::vec4(-7, 15, 10, 1.0), lightMaterial));

	Material lightMaterial2(glm::vec4(0, 0, 0, 400), glm::vec4(0), glm::vec4(0), glm::vec4(1));
	pointLightsVec.push_back(PointLight(glm::vec4(5, 25, -5, 1.0), lightMaterial2));


	//Material sphereMaterial(glm::vec4(0.2, 16, 0, 0), glm::vec4(0.8, 0.3, 0.3, 1.0), glm::vec4(0, 1, 0, 1), glm::vec4(0));
	//const int numX = 2, numY = 4;
	//for (int i = -numX / 2; i < numX / 2; i++) {
	//	for (int j = -numY / 2; j < numY / 2; j++) {
	//		spheresVec.push_back(Sphere(glm::vec4(i, 2, j, 0.4), sphereMaterial));
	//	}
	//}

	spheresVec.push_back(Sphere(glm::vec4(0, 0, 0, 0.2), 
		Material(glm::vec4(0, 16, 0, 0), glm::vec4(1.0, 0.2, 0.4, 1.0), glm::vec4(0.8), glm::vec4(0))));
	spheresVec.push_back(Sphere(glm::vec4(-0.5, -0.5, -0.5, 0.4),
		Material(glm::vec4(0.001, 16, 0, 0), glm::vec4(0.5, 0.1, 0.9, 1.0), glm::vec4(0.9), glm::vec4(0))));
	spheresVec.push_back(Sphere(glm::vec4(-0.5, 0.0, 0.5, 0.3),
		Material(glm::vec4(1, 16, 0, 0), glm::vec4(0.1, 0.7, 0.2, 1.0), glm::vec4(0.5), glm::vec4(0))));
	spheresVec.push_back(Sphere(glm::vec4(0.5, 0.0, -0.5, 0.3),
		Material(glm::vec4(1, 16, 0, 0), glm::vec4(0.3, 0.7, 0.6, 1.0), glm::vec4(0.2), glm::vec4(0))));
	spheresVec.push_back(Sphere(glm::vec4(0.5, 0.5, 0.5, 0.1),
		Material(glm::vec4(1, 16, 0, 0), glm::vec4(0.5, 0.3, 0.2, 1.0), glm::vec4(0.2), glm::vec4(0))));

	spheresVec.push_back(Sphere(glm::vec4(0.5, 0.5, 0.5, 0.1),
		Material(glm::vec4(1, 16, 0, 0), glm::vec4(0.5, 0.3, 0.2, 1.0), glm::vec4(0.2), glm::vec4(0))));

	int quadDim = 2;
	quadsVec.push_back(Quad(glm::vec4(-quadDim, -1, -quadDim, 1), glm::vec4(quadDim, -1, -quadDim, 1), 
		glm::vec4(-quadDim, -1, quadDim, 1), glm::vec4(quadDim, -1, quadDim, 1),
		Material(glm::vec4(1, 16, 0, 0), glm::vec4(1.0, 0.3, 0.3, 1.0), glm::vec4(0.2), glm::vec4(0))));
}

/*
* Create an SSBO for each respective member object type
*/
void Scene::setupComputeShaderData() {

	//
	// UBO's
	//
	unsigned int blockIndexSUBO;
	GLuint bindingIndexSUBO;

	// Point Light UBO
	GLuint pointLightUBO;
	glGenBuffers(1, &pointLightUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, pointLightUBO);
	glBufferData(GL_UNIFORM_BUFFER, pointLightsVec.size() * sizeof(PointLight), &pointLightsVec[0], GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	blockIndexSUBO = glGetUniformBlockIndex(shaders->compShaderID, "pointLightData");
	bindingIndexSUBO = 1;
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndexSUBO, pointLightUBO);
	glUniformBlockBinding(shaders->compShaderID, blockIndexSUBO, bindingIndexSUBO);

	// Sphere UBO
	GLuint sphereUBO;
	glGenBuffers(1, &sphereUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, sphereUBO);
	glBufferData(GL_UNIFORM_BUFFER, spheresVec.size() * sizeof(Sphere), &spheresVec[0], GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	blockIndexSUBO = glGetUniformBlockIndex(shaders->compShaderID, "sphereData");
	bindingIndexSUBO = 2;
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndexSUBO, sphereUBO);
	glUniformBlockBinding(shaders->compShaderID, blockIndexSUBO, bindingIndexSUBO);

	// Plane UBO
	GLuint quadUBO;
	glGenBuffers(1, &quadUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, quadUBO);
	glBufferData(GL_UNIFORM_BUFFER, quadsVec.size() * sizeof(Quad), &quadsVec[0], GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	blockIndexSUBO = glGetUniformBlockIndex(shaders->compShaderID, "quadData");
	bindingIndexSUBO = 3;
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndexSUBO, quadUBO);
	glUniformBlockBinding(shaders->compShaderID, blockIndexSUBO, bindingIndexSUBO);


	//
	// SSBO's
	//

	//GLuint sphereSSBO;
	//glGenBuffers(1, &sphereSSBO);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSSBO);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, spheresVec.size() * sizeof(Sphere), NULL, GL_STATIC_DRAW);
	//struct Sphere* spheres = (struct Sphere*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, spheresVec.size() * sizeof(Sphere), GL_MAP_WRITE_BIT);
	//for (int i = 0; i < spheresVec.size(); i++) {
	//	spheres[i].posRad = spheresVec[i].posRad;
	//	spheres[i].material = spheresVec[i].material;
	//}
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sphereSSBO);
}


void Scene::updateFPS() {
	double timeDiff;

	curTime = glfwGetTime();
	timeDiff = curTime - prevTime;
	counter++;
	if (timeDiff >= 1.0 / 30.0) {
		frameTime = timeDiff / counter;
		std::string FPS = std::to_string((1.0 / timeDiff) * counter);
		std::string ms = std::to_string(frameTime * 1000.0);
		std::string newTitle = "Test - " + FPS + " FPS / " + ms + " ms";
		glfwSetWindowTitle(window, newTitle.c_str());
		prevTime = curTime;
		counter = 0;
	}
}

void Scene::draw() {
	double curTime = glfwGetTime();

	bool resetFrames = false;
	int numAccumFrames = 1; // TODO do something with this

	glBindVertexArray(VAO);

	while (!glfwWindowShouldClose(window)) {
		//break;
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
		updateFPS();
		curTime = glfwGetTime();

		// Draw
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Activate the compute shader and transfer all dynamic scene data
		shaders->activateCompShader();
		glUniform1f(timeLoc, (float)curTime);
		glUniform3fv(cameraPosLoc, 1, glm::value_ptr(camera->position));
		glUniform3fv(cameraDirLoc, 1, glm::value_ptr(camera->direction));

		if (resetFrames) {
			numAccumFrames = 1;
			resetFrames = false;
		}
		glUniform1i(numAccumFramesLoc, numAccumFrames);

		glm::vec4 temp = camera->invProjView * frust00;
		ray00 = glm::vec3(temp) / temp.w;
		temp = camera->invProjView * frust10;
		ray10 = glm::vec3(temp) / temp.w;
		temp = camera->invProjView * frust01;
		ray01 = glm::vec3(temp) / temp.w;
		temp = camera->invProjView * frust11;
		ray11 = glm::vec3(temp) / temp.w;

		glUniform3fv(ray00Loc, 1, glm::value_ptr(ray00));
		glUniform3fv(ray10Loc, 1, glm::value_ptr(ray10));
		glUniform3fv(ray01Loc, 1, glm::value_ptr(ray01));
		glUniform3fv(ray11Loc, 1, glm::value_ptr(ray11));

		glDispatchCompute(COMP_DIM_X, COMP_DIM_Y, 1);
		//glDispatchCompute(TEXTURE_WIDTH, TEXTURE_HEIGHT, 1);

		// make sure writing to image has finished before read
		//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		shaders->activateDefaultShader();
		glBindTextureUnit(0, texID);
		glUniform1i(textureLoc, 0);
		glDrawElements(GL_TRIANGLES, screenQuadInds.size(), GL_UNSIGNED_INT, 0);
		numAccumFrames++;


		glfwSwapBuffers(window);
		glfwPollEvents();


		// Update camera variables
		camera->inputs(frameTime, resetFrames);
		camera->matrix();
	}
}











//#include "scene.h"
//
//Scene::Scene(GLFWwindow* window_) : TEXTURE_WIDTH(512), TEXTURE_HEIGHT(512), COMP_DIM_X(64), COMP_DIM_Y(64) {
//	window = window_;
//	camera = new Camera(window);
//	shaders = new Shader("default.vert", "default.frag", "raytracer.comp");
//
//	setupScreenQuad();
//
//	// Populate scene objects
//	setupSceneObjects();
//	setupComputeShaderData();
//}
//
//Scene::~Scene() {
//	window = nullptr;
//	delete camera;
//	delete shaders;
//}
//
//void Scene::setupScreenQuad() {
//
//	// Shader and screen texture drawing setup
//	glGenTextures(1, &texID);
//	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_2D, texID);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA,
//		GL_FLOAT, NULL);
//
//	glBindImageTexture(0, texID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
//
//
//	GLfloat tempVerts[] = {
//		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
//		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
//		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
//		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
//	};
//
//	GLuint tempInds[] = {
//		0, 2, 1,
//		0, 3, 2
//	};
//
//	screenQuadVerts.assign(&tempVerts[0], &tempVerts[0] + 20);
//	screenQuadInds.assign(&tempInds[0], &tempInds[0] + 6);
//
//	GLuint VBO, EBO;
//	glGenVertexArrays(1, &VAO);
//	glBindVertexArray(VAO);
//	glGenBuffers(1, &VBO);
//	glGenBuffers(1, &EBO);
//
//	// Links the VBO to the VAO
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	// Specifies the layout of the VBO to the VAO
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
//	glEnableVertexAttribArray(1);
//
//	glBufferData(GL_ARRAY_BUFFER, screenQuadVerts.size() * sizeof(GLfloat), screenQuadVerts.data(), GL_STATIC_DRAW);
//
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);	// Binds an element array buffer to the vertex array
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, screenQuadInds.size() * sizeof(GLuint), screenQuadInds.data(), GL_STATIC_DRAW);
//
//	glBindVertexArray(0);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//}
//
///*
//* Create and add all scene objects to local vectors
//*/
//void Scene::setupSceneObjects() {
//	Material material(glm::vec4(0, 0, 64, 0), glm::vec4(0.8, 0.3, 0.3, 1.0), glm::vec4(0.2), glm::vec4(0));
//
//	const int numX = 2, numY = 2;
//	for (int i = -numX / 2; i < numX / 2; i++) {
//		for (int j = -numY / 2; j < numY / 2; j++) {
//			spheresVec.push_back(Sphere(glm::vec4(i, 0, j, 0.4), material));
//		}
//	}
//
//	//spheresVec.push_back(Sphere(glm::vec4(0, 0, 0, 0.2), 
//	//	Material(glm::vec4(0, 0, 16, 0), glm::vec4(1.0, 0.2, 0.4, 1.0), glm::vec4(0.2), glm::vec4(0))));
//	//spheresVec.push_back(Sphere(glm::vec4(-0.5, -0.5, -0.5, 0.4),
//	//	Material(glm::vec4(0, 0, 16, 0), glm::vec4(0.5, 0.1, 0.9, 1.0), glm::vec4(0.2), glm::vec4(0))));
//	//spheresVec.push_back(Sphere(glm::vec4(-0.5, 0.0, 0.5, 0.3),
//	//	Material(glm::vec4(0, 0, 16, 0), glm::vec4(0.1, 0.7, 0.2, 1.0), glm::vec4(0.2), glm::vec4(0))));
//	//spheresVec.push_back(Sphere(glm::vec4(0.5, 0.0, -0.5, 0.3),
//	//	Material(glm::vec4(0, 0, 16, 0), glm::vec4(0.3, 0.7, 0.6, 1.0), glm::vec4(0.2), glm::vec4(0))));
//	//spheresVec.push_back(Sphere(glm::vec4(0.5, 0.5, 0.5, 0.1),
//	//	Material(glm::vec4(0, 0, 16, 0), glm::vec4(0.5, 0.3, 0.2, 1.0), glm::vec4(0.2), glm::vec4(0))));
//
//	//planesVec.push_back(Plane(glm::vec4(-1, -1, -1, 1), glm::vec4(1, -1, -1, 1), glm::vec4(-1, -1, 1, 1), glm::vec4(1, -1, 1, 1),
//	//	Material(glm::vec4(0, 0, 16, 0), glm::vec4(1.0, 0.3, 0.3, 1.0), glm::vec4(0.2), glm::vec4(0))));
//}
//
///*
//* Create an SSBO for each respective member object type
//*/
//void Scene::setupComputeShaderData() {
//	GLuint sphereSSBO;
//	glGenBuffers(1, &sphereSSBO);
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSSBO);
//	glBufferData(GL_SHADER_STORAGE_BUFFER, spheresVec.size() * sizeof(Sphere), NULL, GL_STATIC_DRAW);
//	struct Sphere* spheres = (struct Sphere*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, spheresVec.size() * sizeof(Sphere), GL_MAP_WRITE_BIT);
//	for (int i = 0; i < spheresVec.size(); i++) {
//		spheres[i].posRad = spheresVec[i].posRad;
//		spheres[i].material = spheresVec[i].material;
//	}
//	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sphereSSBO);
//
//
//	GLuint planeSSBO;
//	glGenBuffers(1, &planeSSBO);
//	glBindBuffer(GL_SHADER_STORAGE_BUFFER, planeSSBO);
//	glBufferData(GL_SHADER_STORAGE_BUFFER, planesVec.size() * sizeof(Plane), NULL, GL_STATIC_DRAW);
//	struct Plane* planes = (struct Plane*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, planesVec.size() * sizeof(Plane), GL_MAP_WRITE_BIT);
//	for (int i = 0; i < planesVec.size(); i++) {
//		planes[i].c00 = planesVec[i].c00;
//		planes[i].c10 = planesVec[i].c10;
//		planes[i].c01 = planesVec[i].c01;
//		planes[i].c11 = planesVec[i].c11;
//		planes[i].material = planesVec[i].material;
//	}
//	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, planeSSBO);
//}
//
//
//void Scene::updateFPS() {
//	double timeDiff;
//
//	curTime = glfwGetTime();
//	timeDiff = curTime - prevTime;
//	counter++;
//	if (timeDiff >= 1.0 / 30.0) {
//		frameTime = timeDiff / counter;
//		std::string FPS = std::to_string((1.0 / timeDiff) * counter);
//		std::string ms = std::to_string(frameTime * 1000.0);
//		std::string newTitle = "Test - " + FPS + " FPS / " + ms + " ms";
//		glfwSetWindowTitle(window, newTitle.c_str());
//		prevTime = curTime;
//		counter = 0;
//	}
//}
//
//void Scene::draw() {
//	GLint size;
//	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &size);
//	std::cout << size << std::endl;
//
//	srand(time(0));	
//
//	// Activate the compute shader and transfer all static scene data
//	shaders->activateCompShader();
//	glUniform1i(glGetUniformLocation(shaders->compShaderID, "numSpheres"), spheresVec.size());
//	glUniform1i(glGetUniformLocation(shaders->compShaderID, "numPlanes"), planesVec.size());
//
//	while (!glfwWindowShouldClose(window)) {
//		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) break;
//		updateFPS();
//
//
//		// Draw
//		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//		// Activate the compute shader and transfer all dynamic scene data
//		shaders->activateCompShader();
//		glUniformMatrix4fv(glGetUniformLocation(shaders->compShaderID, "camInvProjView"), 1, GL_FALSE, glm::value_ptr(camera->invProjView));
//		glUniform3fv(glGetUniformLocation(shaders->compShaderID, "cameraPos"), 1, glm::value_ptr(camera->position));
//		glUniform3fv(glGetUniformLocation(shaders->compShaderID, "cameraDir"), 1, glm::value_ptr(camera->direction));
//
//		glm::vec4 temp = camera->invProjView * frust00;
//		ray00 = glm::vec3(temp) / temp.w;
//		temp = camera->invProjView * frust10;
//		ray10 = glm::vec3(temp) / temp.w;
//		temp = camera->invProjView * frust01;
//		ray01 = glm::vec3(temp) / temp.w;
//		temp = camera->invProjView * frust11;
//		ray11 = glm::vec3(temp) / temp.w;
//
//		glUniform3fv(glGetUniformLocation(shaders->compShaderID, "ray00"), 1, glm::value_ptr(ray00));
//		glUniform3fv(glGetUniformLocation(shaders->compShaderID, "ray10"), 1, glm::value_ptr(ray10));
//		glUniform3fv(glGetUniformLocation(shaders->compShaderID, "ray01"), 1, glm::value_ptr(ray01));
//		glUniform3fv(glGetUniformLocation(shaders->compShaderID, "ray11"), 1, glm::value_ptr(ray11));
//
//		glDispatchCompute(COMP_DIM_X, COMP_DIM_Y, 1);
//		//glDispatchCompute(TEXTURE_WIDTH, TEXTURE_HEIGHT, 1);
//
//		// make sure writing to image has finished before read
//		//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
//		glMemoryBarrier(GL_ALL_BARRIER_BITS);
//
//		shaders->activateDefaultShader();
//		glBindTextureUnit(0, texID);
//		glUniform1i(glGetUniformLocation(shaders->screenQuadShaderID, "tex"), 0);
//		glBindVertexArray(VAO);
//		glDrawElements(GL_TRIANGLES, screenQuadInds.size(), GL_UNSIGNED_INT, 0);
//		glBindVertexArray(0);
//
//
//		glfwSwapBuffers(window);
//		glfwPollEvents();
//
//
//		// Update camera variables
//		camera->inputs(frameTime);
//		camera->matrix();
//	}
//}