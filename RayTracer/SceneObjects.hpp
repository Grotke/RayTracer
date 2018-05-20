#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <limits>
#include <cmath>
#include <algorithm>
#include "Color.h"
#include <string>

struct Ray {
	Ray(const glm::vec3& org, const glm::vec3& dir) : origin(org), dir(dir) {}
	glm::vec3 dir;
	glm::vec3 origin;
};

struct Material {
	Color diffuse, specular, emission, ambient;
	float shininess;
	Material(const Color& diffuse, const Color& specular, const Color& emission, const Color& ambient, float shininess): diffuse(diffuse), specular(specular), emission(emission), ambient(ambient), shininess(shininess){}
	Material(const Color& diffuse): diffuse(diffuse){}
	Material(){}
};

struct Light {
	glm::vec4 location;
	Color color;
	bool isPointLight() { return location.w > 0.0f; }
	Light(const glm::vec4& location, const Color& color) : location(location), color(color){}
};

struct Intersection {
	float distAlongRay = std::numeric_limits<float>::infinity();
	Material mat;
	glm::vec3 intersectNormal;
	bool isValidIntersection() const {
		return !glm::isinf(distAlongRay);
	}
	Intersection(float distAlongRay): distAlongRay(distAlongRay){}
	Intersection(float distAlongRay, const glm::vec3& normal) : distAlongRay(distAlongRay), intersectNormal(glm::normalize(normal)) {}
	Intersection(){}
};

struct SceneMetaData {
	std::string filePath;
	std::string sceneTitle;
	SceneMetaData(const std::string& filePath, const std::string& sceneTitle) : filePath(filePath), sceneTitle(sceneTitle) {}
};
