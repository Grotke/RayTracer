#pragma once
#include <glm/glm.hpp>
struct Color {
	int r, g, b;
	int min = 0, max = 255;
	Color(int inR, int inG, int inB) {
		r = glm::clamp(inR, min, max);
		g = glm::clamp(inG, min, max);
		b = glm::clamp(inB, min, max);
	}
	Color(float inR, float inG, float inB) {
		r = glm::clamp((int)(inR * 255), min, max);
		g = glm::clamp((int)(inG * 255), min, max);
		b = glm::clamp((int)(inB * 255), min, max);
	}
	Color(): r(0), g(0), b(0) {}
	Color operator+(const Color& other) const
	{
		return Color(r + other.r , g + other.g , b + other.b);
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
	//Could use a really big number but it's theoretically possible that the intersect 
	//could be very large so I'm using a negative number which should never occur naturally
	float distAlongRay = -1.0f;
	int objectIndex = -1;
	glm::vec3 intersectNormal;
	bool isValidIntersection() { return distAlongRay >= 0.1f; }
	Intersection(float distAlongRay, int objectIndex = -1): distAlongRay(distAlongRay), objectIndex(objectIndex) {}
	Intersection(float distAlongRay, const glm::vec3& normal) : distAlongRay(distAlongRay), intersectNormal(glm::normalize(normal)) {}
	Intersection(){}
};