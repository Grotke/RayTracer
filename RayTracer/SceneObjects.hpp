#pragma once
#include <glm/glm.hpp>
struct Color {
	int r, g, b;
	Color(int r, int g, int b) : r(r), g(g), b(b) {}
	Color(float inR, float inG, float inB) {
		r = inR * 255;
		g = inG * 255;
		b = inB * 255;
	}
	Color() {}
	Color operator+(const Color& other) const
	{
		return Color(r + other.r , g + other.g , b + other.b);
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
	Shape(glm::vec3 center, float radius, const Material& mat) : center(center), radius(radius), isTriangle(false), material(mat) {}
	Shape(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const Material& mat) : v1(v1), v2(v2), v3(v3), isTriangle(true), material(mat) {}
};

struct Light {
	glm::vec4 location;
	Color color;
	Light(const glm::vec4& location, const Color& color) : location(location), color(color){}
};