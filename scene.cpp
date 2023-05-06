
#include "scene.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Scene::Scene(GLFWwindow* window_) : TEXTURE_WIDTH(1024), TEXTURE_HEIGHT(1024), COMP_DIM_X(128), COMP_DIM_Y(128) {
	srand(time(0));

	window = window_;

	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, keyInputSetup);

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

void Scene::keyInput(int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_M:
			if (randmode == 0) randmode = 1;
			else randmode = 0;
			resetFrames = true;
			std::cout << "Draw Frustum: " << ((randmode) ? "Rand 2" : "Rand 1") << std::endl;
			break;
		default:
			break;
		}
	}
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
	// RANDOM BALLS SCENE
	//Material lightMaterial(glm::vec4(0, 0, 0, 300), glm::vec4(0), glm::vec4(0), glm::vec4(1));
	//pointLightsVec.push_back(PointLight(glm::vec4(-7, 15, 10, 1.0), lightMaterial));

	//Material lightMaterial2(glm::vec4(0, 0, 0, 1000), glm::vec4(0), glm::vec4(0), glm::vec4(0), glm::vec4(1));
	//pointLightsVec.push_back(PointLight(glm::vec4(5, 25, -5, 1.0), lightMaterial2));

	const int numX = 4, numY = 4;
	for (int i = -numX / 2; i < numX / 2; i++) {
		for (int j = -numY / 2; j < numY / 2; j++) {
			spheresVec.push_back(Sphere(
				glm::vec4((float)(rand()) / (float)(RAND_MAX) * 10.0 - 5.0, (float)(rand()) / (float)(RAND_MAX) * 10.0 - 5.0, (float)(rand()) / (float)(RAND_MAX) * 10.0 - 5.0,
					(float)(rand()) / (float)(RAND_MAX) + 0.2), 
				Material(glm::vec4((float)(rand()) / (float)(RAND_MAX), 0, ((float)(rand()) / (float)(RAND_MAX) + 1.0) * (((float)(rand()) / (float)(RAND_MAX)) > 0.6), 0),
						glm::vec4((float)(rand()) / (float)(RAND_MAX), (float)(rand()) / (float)(RAND_MAX), (float)(rand()) / (float)(RAND_MAX), 1.0),
						glm::vec4((float)(rand()) / (float)(RAND_MAX), (float)(rand()) / (float)(RAND_MAX), (float)(rand()) / (float)(RAND_MAX), 1),
						glm::vec4((float)(rand()) / (float)(RAND_MAX), (float)(rand()) / (float)(RAND_MAX), (float)(rand()) / (float)(RAND_MAX), 1), glm::vec4(0))));
		}
	}



	// SUNSET SCENE
	Material sunset(glm::vec4(0, 0, 0, 100), glm::vec4(0), glm::vec4(0), glm::vec4(0), glm::vec4(1, 0.5, 0.5, 1.0));
	spheresVec.push_back(Sphere(glm::vec4(40, 5, 50, 10.0), sunset));

	//Material sunset2(glm::vec4(0, 0, 0, 2000), glm::vec4(0), glm::vec4(0), glm::vec4(0), glm::vec4(1, 0.5, 0.5, 1.0));
	//pointLightsVec.push_back(PointLight(glm::vec4(-5, 50, -5, 1.0), sunset2));

	//spheresVec.push_back(Sphere(glm::vec4(0, 0, 0, 0.2), 
	//	Material(glm::vec4(0, 0, 0, 0), glm::vec4(1), glm::vec4(0.8), glm::vec4(0), glm::vec4(0))));
	////spheresVec.push_back(Sphere(glm::vec4(-0.5, -0.5, -0.5, 0.4),
	////	Material(glm::vec4(0.001, 16, 0, 0), glm::vec4(247/255.0, 217/255.0, 45/255.0, 1.0), glm::vec4(0.9), glm::vec4(0), glm::vec4(0))));
	////spheresVec.push_back(Sphere(glm::vec4(-0.5, 0.0, 0.5, 0.3),
	////	Material(glm::vec4(1, 16, 0, 0), glm::vec4(247 / 255.0, 45 / 255.0, 109 / 255.0, 1.0), glm::vec4(0.5), glm::vec4(0), glm::vec4(0))));
	////spheresVec.push_back(Sphere(glm::vec4(0.5, 0.0, -0.5, 0.3),
	////	Material(glm::vec4(1, 16, 0, 0), glm::vec4(0.3, 0.4, 0.4, 1.0), glm::vec4(0.2), glm::vec4(0), glm::vec4(0))));
	////spheresVec.push_back(Sphere(glm::vec4(0.5, 0.5, 0.5, 0.1),
	////	Material(glm::vec4(1, 16, 0, 0), glm::vec4(0.5, 0.3, 0.2, 1.0), glm::vec4(0.2), glm::vec4(0), glm::vec4(0))));

	//Material glass(glm::vec4(0, 0, 1.5, 0), glm::vec4(1), glm::vec4(1), glm::vec4(1), glm::vec4(0));
	//spheresVec.push_back(Sphere(glm::vec4(1, 0, 1, 0.4), glass));

	//int quadDim = 2;
	//int quadYpos = -1;
	//Material quadMat(glm::vec4(0.5, 0, 0, 0), glm::vec4(1), glm::vec4(0.2), glm::vec4(0), glm::vec4(0));
	//quadsVec.push_back(Quad(glm::vec4(-quadDim, quadYpos, -quadDim, 1), glm::vec4(quadDim, quadYpos, -quadDim, 1),
	//	glm::vec4(-quadDim, quadYpos, quadDim, 1), glm::vec4(quadDim, quadYpos, quadDim, 1),
	//	quadMat));


	// GREYSCALE SCENE
	//Material matteGrey(glm::vec4(0, 0, 0, 0), glm::vec4(1), glm::vec4(1), glm::vec4(0), glm::vec4(0));
	//Material red(glm::vec4(0, 0, 0, 0), glm::vec4(1, 0, 0, 1), glm::vec4(0), glm::vec4(0), glm::vec4(0));
	//Material green(glm::vec4(0, 0, 0, 0), glm::vec4(0, 1, 0, 1), glm::vec4(0), glm::vec4(0), glm::vec4(0));
	//Material blue(glm::vec4(0, 0, 0, 0), glm::vec4(0, 0, 1, 1), glm::vec4(0), glm::vec4(0), glm::vec4(0));
	//Material shinyGrey(glm::vec4(0.98, 0, 0, 0), glm::vec4(1), glm::vec4(0), glm::vec4(0), glm::vec4(0));
	//Material clearGrey(glm::vec4(1, 0, 1.5, 0), glm::vec4(1), glm::vec4(0), glm::vec4(1), glm::vec4(0));
	//spheresVec.push_back(Sphere(glm::vec4(1, 0, 0, 0.3), clearGrey));
	//spheresVec.push_back(Sphere(glm::vec4(0, 0, 1, 0.3), matteGrey));
	//spheresVec.push_back(Sphere(glm::vec4(0.7, 0, 0.7, 0.3), blue));

	//float quadDim = 2;
	//float quadYpos = -0.301;
	//quadsVec.push_back(Quad(glm::vec4(-quadDim, quadYpos, -quadDim, 1), glm::vec4(quadDim, quadYpos, -quadDim, 1),
	//	glm::vec4(-quadDim, quadYpos, quadDim, 1), glm::vec4(quadDim, quadYpos, quadDim, 1),
	//	matteGrey));

	////quadsVec.push_back(Quad(glm::vec4(2, 1.6, 2, 1), glm::vec4(-2, 1.6, 2, 1), glm::vec4(2, quadYpos, 2, 1), glm::vec4(-2, quadYpos, 2, 1),
	////	matteGrey));
	////quadsVec.push_back(Quad(glm::vec4(2, 1.6, -2, 1), glm::vec4(-2, 1.6, -2, 1), glm::vec4(2, quadYpos, -2, 1), glm::vec4(-2, quadYpos, -2, 1),
	////	matteGrey));
	////quadsVec.push_back(Quad(glm::vec4(2, 1.6, -2, 1), glm::vec4(2, 1.6, 2, 1), glm::vec4(2, quadYpos, -2, 1), glm::vec4(2, quadYpos, 2, 1),
	////	shinyGrey));

	//Material emissive(glm::vec4(0, 0, 0, 5), glm::vec4(0), glm::vec4(0), glm::vec4(0), glm::vec4(1));
	//spheresVec.push_back(Sphere(glm::vec4(-0.5, 0.5, -0.5, 0.6), emissive));


	// CORNELL BOX
	//Material white(glm::vec4(1, 0, 0, 0), glm::vec4(1), glm::vec4(0), glm::vec4(0), glm::vec4(0));
	//Material green(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 1), glm::vec4(0), glm::vec4(0), glm::vec4(0));
	//Material red(glm::vec4(1, 0, 0, 0), glm::vec4(1, 0, 0, 1), glm::vec4(0), glm::vec4(0), glm::vec4(0));
	//Material light(glm::vec4(1, 0, 0, 50), glm::vec4(0), glm::vec4(0), glm::vec4(0), glm::vec4(1));

	//quadsVec.push_back(Quad(glm::vec4(-1, 1, 1, 1), glm::vec4(1, 1, 1, 1), glm::vec4(-1, 1, -1, 1), glm::vec4(1, 1, -1, 1), white));
	//quadsVec.push_back(Quad(glm::vec4(-1, 1, -1, 1), glm::vec4(1, 1, -1, 1), glm::vec4(-1, -1, -1, 1), glm::vec4(1, -1, -1, 1), white));
	//quadsVec.push_back(Quad(glm::vec4(-1, -1, -1, 1), glm::vec4(1, -1, -1, 1), glm::vec4(-1, -1, 1, 1), glm::vec4(1, -1, 1, 1), white));
	//quadsVec.push_back(Quad(glm::vec4(1, 1, -1, 1), glm::vec4(1, 1, 1, 1), glm::vec4(1, -1, -1, 1), glm::vec4(1, -1, 1, 1), green));
	//quadsVec.push_back(Quad(glm::vec4(-1, 1, 1, 1), glm::vec4(-1, 1, -1, 1), glm::vec4(-1, -1, 1, 1), glm::vec4(-1, -1, -1, 1), red));
	//quadsVec.push_back(Quad(glm::vec4(-0.2, 0.99, 0.2, 1), glm::vec4(0.2, 0.99, 0.2, 1), glm::vec4(-0.2, 0.99, -0.2, 1), glm::vec4(0.2, 0.99, -0.2, 1), light));

	// Junk objects
	pointLightsVec.push_back(PointLight());
	//spheresVec.push_back(Sphere());
	quadsVec.push_back(Quad());
}

/*
* Create an SSBO for each respective member object type
*/
void Scene::setupComputeShaderData() {

	int sbWidth, sbHeight, sbChannels;
	float* skyboxData = stbi_loadf("sunset_in_the_chalk_quarry_2k.hdr", &sbWidth, &sbHeight, &sbChannels, 0);
	// Skybox
	glGenTextures(1, &skyboxID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, skyboxID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sbWidth, sbHeight, 0, GL_RGB, GL_FLOAT, skyboxData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindImageTexture(1, skyboxID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	stbi_image_free(skyboxData);



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
	bindingIndexSUBO = 5;
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
	bindingIndexSUBO = 6;
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
	bindingIndexSUBO = 7;
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

	int numAccumFrames = 0;

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
		
		glUniform1i(glGetUniformLocation(shaders->compShaderID, "randMode"), randmode);

		if (resetFrames) {
			numAccumFrames = 0;
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