#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <limits>
struct Color {
	float r, g, b;
	int min = 0, max = 255;
	Color(float inR, float inG, float inB) : r(inR), g(inG), b(inB) {
	}

	Color(): r(0.0f), g(0.0f), b(0.0f) {}

	int getR() {
		return glm::clamp((int)(r * 255), min, max);
	}

	int getG() {
		return glm::clamp((int)(g * 255), min, max);
	}

	int getB() {
		return glm::clamp((int)(b * 255), min, max);
	}

	Color operator+(const Color& other) const
	{
		return Color(r + other.r , g + other.g , b + other.b);
	}

	Color& operator+=(const Color& color2) {
		this->r += color2.r;
		this->g += color2.g;
		this->b += color2.b;
		return *this;
	}

	glm::vec3 getAsFloat() const {
		return glm::vec3(r / (float)max, g / (float)max, b / (float)max);
	}
};

struct Material {
	Color diffuse, specular, emission, ambient;
	float shininess;
	Material(const Color& diffuse, const Color& specular, const Color& emission, const Color& ambient, float shininess): diffuse(diffuse), specular(specular), emission(emission), ambient(ambient), shininess(shininess){}
	Material(const Color& diffuse): diffuse(diffuse){}
};

struct Shape {
	bool isTriangle;
	glm::vec3 v1, v2, v3;
	glm::vec3 n1, n2, n3;
	const glm::vec3 center;
	float radius;
	const Material material;
	glm::mat4 transform;
	void transformVerts() {
		v1 = transform * glm::vec4(v1, 1.0f);
		v2 = transform * glm::vec4(v2, 1.0f);
		v3 = transform * glm::vec4(v3, 1.0f);
	}
	void transformNorms() {
		glm::mat4 inverseTranspose = glm::inverse(glm::transpose(transform));
		n1 = inverseTranspose * glm::vec4(n1, 0.0f);
		n2 = inverseTranspose * glm::vec4(n2, 0.0f);
		n3 = inverseTranspose * glm::vec4(n3, 0.0f);
	}
	void print() {
		std::cout << "vertex #1 x: " << v1.x << " y: " << v1.y << " z: " << v1.z << std::endl;
		std::cout << "vertex #2 x: " << v2.x << " y: " << v2.y << " z: " << v2.z << std::endl;
		std::cout << "vertex #3 x: " << v3.x << " y: " << v3.y << " z: " << v3.z << std::endl;
	}
	Shape(glm::vec3 center, float radius, const Material& mat) : center(center), radius(radius), isTriangle(false), material(mat) {}
	Shape(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const Material& mat) : v1(v1), v2(v2), v3(v3), isTriangle(true), material(mat) {}
};

struct Light {
	glm::vec4 location;
	Color color;
	bool isPointLight() { return location.w > 0.0f; }
	Light(const glm::vec4& location, const Color& color) : location(location), color(color){}
};

struct Intersection {
	float distAlongRay = std::numeric_limits<float>::infinity();
	int objectIndex = -1;
	glm::vec3 intersectNormal;
	bool isValidIntersection() {
		return !glm::isinf(distAlongRay);
	}
	Intersection(float distAlongRay, int objectIndex = -1): distAlongRay(distAlongRay), objectIndex(objectIndex) {}
	Intersection(float distAlongRay, const glm::vec3& normal) : distAlongRay(distAlongRay), intersectNormal(glm::normalize(normal)) {}
	Intersection(){}
};

struct SceneMetaData {
	std::string filePath;
	std::string sceneTitle;
	SceneMetaData(const std::string& filePath, const std::string& sceneTitle) : filePath(filePath), sceneTitle(sceneTitle) {}
};