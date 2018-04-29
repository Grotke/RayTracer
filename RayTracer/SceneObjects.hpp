#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <limits>
#include <cmath>
#include <algorithm>
#include "Camera.h"

enum class Axis {
	X,
	Y,
	Z
};

struct Ray {
	Ray(const glm::vec3& org, const glm::vec3& dir) : origin(org), dir(glm::normalize(dir)) {}
	glm::vec3 dir;
	glm::vec3 origin;
};

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
	bool isValidIntersection() const {
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

//Axis aligned bounding box
struct AABB {
	glm::vec3 min = glm::vec3(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
	glm::vec3 max = glm::vec3(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
	AABB splitLeft() {
		Axis longest = getLongestAxis();
		glm::vec3 midpoint = getMidPoint();
		glm::vec3 newMax;
		switch (longest) {
			case Axis::X:
				newMax = glm::vec3(midpoint.x, max.y, max.z);
			case Axis::Y:
				newMax = glm::vec3(max.x, midpoint.y, max.z);
			case Axis::Z:
				newMax = glm::vec3(max.x, max.y, midpoint.z);
		}
		return AABB(min, newMax);
	}

	AABB splitRight() {
		Axis longest = getLongestAxis();
		glm::vec3 midpoint = getMidPoint();
		glm::vec3 newMin;
		switch (longest) {
		case Axis::X:
			newMin = glm::vec3(midpoint.x, min.y, min.z);
		case Axis::Y:
			newMin = glm::vec3(min.x, midpoint.y, min.z);
		case Axis::Z:
			newMin = glm::vec3(min.x, min.y, midpoint.z);
		}
		return AABB(newMin, max);
	}

	void expand(const Shape& obj) {
		if (obj.getMinX() < min.x) { min.x = obj.getMinX(); }
		if (obj.getMinY() < min.y) { min.y = obj.getMinY(); }
		if (obj.getMinZ() < min.z) { min.z = obj.getMinZ(); }
		if (obj.getMaxX() > max.x) { max.x = obj.getMaxX(); }
		if (obj.getMaxY() > max.y) { max.y = obj.getMaxY(); }
		if (obj.getMaxZ() > max.z) { max.z = obj.getMaxZ(); }
	}

	bool intersect(const Ray& ray) const {
			// Any component of direction could be 0!
			// Address this by using a small number, close to
			// 0 in case any of directions components are 0
			float t1 = (min.x - ray.origin.x) / (glm::epsilonEqual(ray.dir.x, 0.0f, 0.0001f) ? 0.00001f : ray.dir.x);
			float t2 = (max.x - ray.origin.x) / (glm::epsilonEqual(ray.dir.x, 0.0f, 0.0001f) ? 0.00001f : ray.dir.x);
			float t3 = (min.y - ray.origin.y) / (glm::epsilonEqual(ray.dir.y, 0.0f, 0.0001f) ? 0.00001f : ray.dir.y);
			float t4 = (max.y - ray.origin.y) / (glm::epsilonEqual(ray.dir.y, 0.0f, 0.0001f) ? 0.00001f : ray.dir.y);
			float t5 = (min.z - ray.origin.z) / (glm::epsilonEqual(ray.dir.z, 0.0f, 0.0001f) ? 0.00001f : ray.dir.z);
			float t6 = (max.z - ray.origin.z) / (glm::epsilonEqual(ray.dir.z, 0.0f, 0.0001f) ? 0.00001f : ray.dir.z);

			float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
			float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

			// if tmax < 0, ray is intersecting AABB
			// but entire AABB is behing it's origin
			if (tmax < 0) {
				return false;
			}

			// if tmin > tmax, ray doesn't intersect AABB
			if (tmin > tmax) {
				return false;
			}
			return true;
	}

	bool contains(const Shape& obj) const {
		return obj.isInside(*this);
	}

	bool contains(const glm::vec3& point) const {
		return point.x <= max.x && point.x >= min.x && point.y <= max.y && point.y >= min.y && point.z <= max.z && point.z >= min.z;
	}

	Axis getLongestAxis() const {
		float maxAxis = std::max(max.z, std::max(max.x, max.y));
		if (max.z == maxAxis) {
			return Axis::Z;
		}
		else if (max.y == maxAxis) {
			return Axis::Y;
		}
		return Axis::X;
	}

	glm::vec3 getMidPoint() const {
		return max - min / 2.0f;
	}
	AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}
	AABB() {}
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
	void print() const {
		std::cout << "vertex #1 x: " << v1.x << " y: " << v1.y << " z: " << v1.z << std::endl;
		std::cout << "vertex #2 x: " << v2.x << " y: " << v2.y << " z: " << v2.z << std::endl;
		std::cout << "vertex #3 x: " << v3.x << " y: " << v3.y << " z: " << v3.z << std::endl;
	}

	float getMinX() const {
		if (isTriangle) {
			return std::min(v3.x, std::min(v1.x, v2.x));
		}
		return center.x - radius;
	}

	float getMinY() const {
		if (isTriangle) {
			return std::min(v3.y, std::min(v1.y, v2.y));
		}
		return center.y - radius;
	}

	float getMinZ() const {
		if (isTriangle) {
			return std::min(v3.z, std::min(v1.z, v2.z));
		}
		return center.z - radius;
	}

	float getMaxX() const {
		if (isTriangle) {
			return std::max(v3.x, std::max(v1.x, v2.x));
		}
		return center.x + radius;
	}

	float getMaxY() const {
		if (isTriangle) {
			return std::max(v3.y, std::max(v1.y, v2.y));
		}
		return center.y + radius;
	}

	float getMaxZ() const {
		if (isTriangle) {
			return std::max(v3.z, std::max(v1.z, v2.z));
		}
		return center.z + radius;
	}

	bool isInside(const AABB& box) const {
		if (isTriangle) {
			return TriangleAABBCollide(*this, box);
		}
		float x = std::max(box.min.x, std::min(center.x, box.max.x));
		float y = std::max(box.min.y, std::min(center.y, box.max.y));
		float z = std::max(box.min.z, std::min(center.z, box.max.z));

		// this is the same as isPointInsideSphere
		float distance = std::sqrt((x - center.x) * (x - center.x) +
			(y - center.y) * (y - center.y) +
			(z - center.z) * (z - center.z));
		//https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection says doesn't work when only colliding on face
		return box.contains(center) || box.contains(glm::normalize(box.getMidPoint() - center) * radius + center) || radius > glm::length(box.getMidPoint() - center) || distance < radius;
	}

	//https://github.com/gszauer/GamePhysicsCookbook/blob/master/Code/Geometry3D.cpp
	bool TriangleAABBCollide(const Shape& t, const AABB& a) const {
		// Compute the edge vectors of the triangle  (ABC)
		glm::vec3 f0 = t.v2 - t.v1;
		glm::vec3 f1 = t.v3 - t.v2;
		glm::vec3 f2 = t.v1 - t.v3;

		// Compute the face normals of the AABB
		glm::vec3 u0(1.0f, 0.0f, 0.0f);
		glm::vec3 u1(0.0f, 1.0f, 0.0f);
		glm::vec3 u2(0.0f, 0.0f, 1.0f);

		glm::vec3 test[13] = {
			// 3 Normals of AABB
			u0, // AABB Axis 1
			u1, // AABB Axis 2
			u2, // AABB Axis 3
				// 1 Normal of the Triangle
				glm::cross(f0, f1),
				// 9 Axis, cross products of all edges
				glm::cross(u0, f0),
				glm::cross(u0, f1),
				glm::cross(u0, f2),
				glm::cross(u1, f0),
				glm::cross(u1, f1),
				glm::cross(u1, f2),
				glm::cross(u2, f0),
				glm::cross(u2, f1),
				glm::cross(u2, f2)
		};

		for (int i = 0; i < 13; ++i) {
			if (!OverlapOnAxis(a, t, test[i])) {
				return false; // Seperating axis found
			}
		}

		return true; // Seperating axis not found
	}

	bool OverlapOnAxis(const AABB& aabb, const Shape& triangle, const glm::vec3& axis) const {
		glm::vec2 a = GetInterval(aabb, axis);
		glm::vec2 b = GetInterval(triangle, axis);
		return ((b.x <= a.y) && (a.x <= b.y));
	}

	//X = min, Y = max
	glm::vec2 GetInterval(const Shape& triangle, const glm::vec3& axis) const {
		glm::vec2 result;

		result.x = glm::dot(axis, triangle.v1);
		result.y = result.x;

		float value = glm::dot(axis, triangle.v2);
		result.x = std::min(result.x, value);
		result.y = std::max(result.y, value);
		value = glm::dot(axis, triangle.v3);
		result.x = std::min(result.x, value);
		result.y = std::max(result.y, value);

		return result;
	}

	glm::vec2 GetInterval(const AABB& aabb, const glm::vec3& axis) const {
		glm::vec3 i = aabb.min;
		glm::vec3 a = aabb.max;

		glm::vec3 vertex[8] = {
			glm::vec3(i.x, a.y, a.z),
			glm::vec3(i.x, a.y, i.z),
			glm::vec3(i.x, i.y, a.z),
			glm::vec3(i.x, i.y, i.z),
			glm::vec3(a.x, a.y, a.z),
			glm::vec3(a.x, a.y, i.z),
			glm::vec3(a.x, i.y, a.z),
			glm::vec3(a.x, i.y, i.z)
		};

		glm::vec2 result;
		result.x = result.y = glm::dot(axis, vertex[0]);

		for (int i = 1; i < 8; ++i) {
			float projection = glm::dot(axis, vertex[i]);
			result.x = (projection < result.x) ? projection : result.x;
			result.y = (projection > result.y) ? projection : result.y;
		}

		return result;
	}

	Intersection intersectTriangle(const Ray& ray) const {
		glm::vec3 e1 = v3 - v2;
		glm::vec3 e2 = v1 - v3;
		glm::vec3 e3 = v2 - v1;
		glm::vec3 planeNormal = glm::cross(e1, e2);
		if (!glm::epsilonEqual(glm::dot(ray.dir, planeNormal), 0.0f, 0.01f)) {
			float d = glm::dot(v1, planeNormal);
			float t = (d - glm::dot(ray.origin, planeNormal)) / glm::dot(ray.dir, planeNormal);
			if (t < 0.005f) {
				return Intersection();
			}
			glm::vec3 P = ray.origin + t * ray.dir;
			glm::vec3 d1 = P - v1;
			glm::vec3 d2 = P - v2;
			glm::vec3 d3 = P - v3;
			float totalArea = glm::dot(glm::cross(e1, e2), planeNormal) / 2;
			bool b0 = (glm::dot(glm::cross(e1, d3), planeNormal) / 2 / totalArea) >= 0;
			bool b1 = (glm::dot(glm::cross(e2, d1), planeNormal) / 2 / totalArea) >= 0;
			bool b2 = (glm::dot(glm::cross(e3, d2), planeNormal) / 2 / totalArea) >= 0;
			if (b0 && b1 && b2) {
				return Intersection(glm::distance(P, ray.origin), planeNormal);
			}
		}
		return Intersection();
	}

	float calculateDiscriminant(float a, float b, float c) const {
		return glm::pow(b, 2) - (4 * a*c);
	}

	Intersection intersectSphere(const Ray& rawRay) const {
		Ray ray(glm::inverse(transform) * glm::vec4(rawRay.origin, 1.0f), glm::normalize(glm::inverse(transform) * glm::vec4(rawRay.dir, 0.0f)));
		float a = glm::dot(ray.dir, ray.dir);
		float b = 2.0f * glm::dot(ray.dir, (ray.origin - center));
		float c = glm::dot((ray.origin - center), (ray.origin - center)) - glm::pow(radius, 2.0f);
		float discrim = calculateDiscriminant(a, b, c);
		if (discrim < 0.0f) {
			return Intersection();
		}
		float x1 = (-b + glm::sqrt(discrim)) / 2.0f * a;
		float x2 = (-b - glm::sqrt(discrim)) / 2.0f * a;
		float t = std::min(x1, x2);
		if (t < 0.005f) {
			//One is negative so return the maximum number which should be positive
			return Intersection();
		}
		glm::vec3 transfPoint = ray.origin + ray.dir * t;
		glm::vec3 normal = glm::transpose(glm::inverse(transform)) * glm::vec4(transfPoint - center, 0.0f);

		glm::vec3 finalPoint = transform * glm::vec4(transfPoint, 1.0f);
		return Intersection(glm::distance(finalPoint, rawRay.origin), normal);
	}

	Intersection intersect(const Ray& ray) const {
		if (isTriangle) {
			return intersectTriangle(ray);
		}
		return intersectSphere(ray);
	}

	Shape(glm::vec3 center, float radius, const Material& mat) : center(center), radius(radius), isTriangle(false), material(mat) {}
	Shape(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const Material& mat) : v1(v1), v2(v2), v3(v3), isTriangle(true), material(mat) {}
};