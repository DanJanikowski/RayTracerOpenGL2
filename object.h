#pragma once

#include <glm/glm.hpp>


/*
	IMPORTANT NOTE
	- These data structures are sent to GPU using an SSBO
	- Specifications with memory alignment create trouble when using vec3 thus only vec4/ivec4 will be used (4th component is 1.0 for all extended vec3's)
	- glm::vec4 require being aligned on 16 byte boundaries
	- If any member data breaks this alignment then an 'alignas(16)' qualifier should be placed before the first vec4 member
	- ESSENTIALLY sizeof all structs should be a multiple of 16
		- alignas changes the sizeof a struct
		- Member functions do NOT
*/

struct Material {
	Material();
	Material(glm::vec4 data_, glm::vec4 diffuseColor_, glm::vec4 specularColor_, glm::vec4 refractionColor_, glm::vec4 emissionColor_);
	~Material();

	// Stores material data: 
	// data.x - smoothness = how distrubed the reflectance rays are/percentage chance to reflect/absorb
	// data.y - glossiness
	// data.z - index of refraction
	// data.w - lightPower = if the material type is a light defines the lights power otherwise 0
	// 
	// 
	// 
	// 
	// OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD 
	// data.x - material type = (normal mat, light, dielectric as an integer 0, 1, 2...)
	//					0 = 
	//					1 = 
	//					2 = 
	// data.y - roughness = how distrubed the reflectance rays are/percentage chance to reflect/absorb
	// data.z - shininess = blinn phong exponent
	// data.w - lightPower = if the material type is a light defines the lights power otherwise 0
	// OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD




	glm::vec4 data;
	//TODO have to add refractive index for dielectrics

	glm::vec4 diffuseColor;		// vec4(0) if the material type IS an emitter
	glm::vec4 glossColor;	// vec4(0) if the material type IS an emitter
	glm::vec4 refractionColor;
	glm::vec4 emissionColor;	// vec4(0) if the material type is NOT an emitter
};


// All object types are individual structs
// Each object type is passed to the shader as a separate SSBO

struct PointLight {
	PointLight();
	PointLight(glm::vec4 pos_, Material material_);
	~PointLight();

	glm::vec4 pos;
	Material material;
};

struct Sphere {
	Sphere();
	Sphere(glm::vec4 posRad_, Material material_);
	~Sphere();

	// posRad.xyz = sphere position
	// posRad.w = sphere radius
	glm::vec4 posRad;
	Material material;
};

struct Triangle {
	Triangle();
	Triangle(glm::vec4 p0_, glm::vec4 p1_, glm::vec4 p2_, Material material_);
	~Triangle();

	glm::vec4 p0;
	glm::vec4 p1;
	glm::vec4 p2;
	Material material;
};

struct Quad {
	Quad();
	Quad(glm::vec4 c00_, glm::vec4 c10_, glm::vec4 c01_, glm::vec4 c11_, Material material_);
	~Quad();

	// Plane corner vertices
	glm::vec4 c00;
	glm::vec4 c10;
	glm::vec4 c01;
	glm::vec4 c11;
	Material material;
};