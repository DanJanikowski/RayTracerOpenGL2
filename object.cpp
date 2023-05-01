
#include "object.h"



Material::Material() {
	data = glm::vec4(0);
	diffuseColor = glm::vec4(0);
	specularColor = glm::vec4(0);
	emissionColor = glm::vec4(0);
}
Material::Material(glm::vec4 data_, glm::vec4 diffuseColor_, glm::vec4 specularColor_, glm::vec4 emissionColor_) {
	data = data_;
	diffuseColor = diffuseColor_;
	specularColor = specularColor_;
	emissionColor = emissionColor_;
}
Material::~Material() {}

//========================================================

PointLight::PointLight() {
	material = Material();
}
PointLight::PointLight(Material material_) {
	material = material_;
}
PointLight::~PointLight() {}

//========================================================

Sphere::Sphere() {
	posRad = glm::vec4(0);
	material = Material();
}
Sphere::Sphere(glm::vec4 posRad_, Material material_) {
	posRad = posRad_;
	material = material_;
}
Sphere::~Sphere() {}

//========================================================

Triangle::Triangle() {
	p0 = glm::vec4(0);
	p1 = glm::vec4(0);
	p2 = glm::vec4(0);
	material = Material();
}
Triangle::Triangle(glm::vec4 p0_, glm::vec4 p1_, glm::vec4 p2_, Material material_) {
	p0 = p0_;
	p1 = p1_;
	p2 = p2_;
	material = Material();

}
Triangle::~Triangle() {}

//========================================================

Plane::Plane() {
	c00 = glm::vec4(0);
	c10 = glm::vec4(0);
	c01 = glm::vec4(0);
	c11 = glm::vec4(0);
	material = Material();
}
Plane::Plane(glm::vec4 c00_, glm::vec4 c10_, glm::vec4 c01_, glm::vec4 c11_, Material material_) {
	c00 = c00_;
	c10 = c10_;
	c01 = c01_;
	c11 = c11_;
	material = material_;
}
Plane::~Plane() {}