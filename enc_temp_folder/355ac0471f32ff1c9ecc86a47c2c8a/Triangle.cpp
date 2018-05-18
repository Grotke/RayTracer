#include "Triangle.h"
#include "AABB.h"
#include <algorithm>
#include <glm/gtc/epsilon.hpp>



Triangle::Triangle(const glm::vec3& inV1, const glm::vec3& inV2, const glm::vec3& inV3, const glm::mat4& inTransform, const Material& mat, const glm::vec3& inN1, const glm::vec3& inN2, const glm::vec3& inN3)
	:Shape(inTransform, mat)
{
	v1 = transform * glm::vec4(inV1, 1.0f);
	v2 = transform * glm::vec4(inV2, 1.0f);
	v3 = transform * glm::vec4(inV3, 1.0f);
	glm::mat4 inverseTranspose = glm::inverse(glm::transpose(transform));
	n1 = inverseTranspose * glm::vec4(inN1, 0.0f);
	n2 = inverseTranspose * glm::vec4(inN2, 0.0f);
	n3 = inverseTranspose * glm::vec4(inN3, 0.0f);
}


Triangle::~Triangle()
{
}

Triangle::Triangle(const glm::vec3& inV1, const glm::vec3& inV2, const glm::vec3& inV3, const glm::mat4& inTransform, const Material& mat) :Shape(inTransform, mat) {
	v1 = transform * glm::vec4(inV1, 1.0f);
	v2 = transform * glm::vec4(inV2, 1.0f);
	v3 = transform * glm::vec4(inV3, 1.0f);
	glm::mat4 inverseTranspose = glm::inverse(glm::transpose(transform));
	glm::vec3 planeNormal = calculatePlaneNormal();
	n1 = inverseTranspose * glm::vec4(planeNormal, 0.0f);
	n2 = inverseTranspose * glm::vec4(planeNormal, 0.0f);
	n3 = inverseTranspose * glm::vec4(planeNormal, 0.0f);
}

float Triangle::getMinX() const {
	return std::min(v3.x, std::min(v1.x, v2.x));
}
float Triangle::getMinY() const {
	return std::min(v3.y, std::min(v1.y, v2.y));
}
float Triangle::getMinZ() const {
	return std::min(v3.z, std::min(v1.z, v2.z));
}
float Triangle::getMaxX() const {
	return std::max(v3.x, std::max(v1.x, v2.x));
}
float Triangle::getMaxY() const {
	return std::max(v3.y, std::max(v1.y, v2.y));
}
float Triangle::getMaxZ() const  {
	return std::max(v3.z, std::max(v1.z, v2.z));
}

//https://github.com/gszauer/GamePhysicsCookbook/blob/master/Code/Geometry3D.cpp
bool Triangle::isInside(const AABB& box) const {
	// Compute the edge vectors of the triangle  (ABC)
	glm::vec3 f0 = v2 - v1;
	glm::vec3 f1 = v3 - v2;
	glm::vec3 f2 = v1 - v3;

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
		if (!OverlapOnAxis(box, test[i])) {
			return false; // Seperating axis found
		}
	}

	return true; // Seperating axis not found
}

bool Triangle::OverlapOnAxis(const AABB& box, const glm::vec3& axis) const {
	glm::vec2 a = GetInterval(box, axis);
	glm::vec2 b = GetInterval(axis);
	return ((b.x <= a.y) && (a.x <= b.y));
}

//X = min, Y = max
glm::vec2 Triangle::GetInterval(const glm::vec3& axis) const {
	glm::vec2 result;

	result.x = glm::dot(axis, v1);
	result.y = result.x;

	float value = glm::dot(axis, v2);
	result.x = std::min(result.x, value);
	result.y = std::max(result.y, value);
	value = glm::dot(axis, v3);
	result.x = std::min(result.x, value);
	result.y = std::max(result.y, value);

	return result;
}

glm::vec2 Triangle::GetInterval(const AABB& aabb, const glm::vec3& axis) const {
	glm::vec3 i = aabb.getMin();
	glm::vec3 a = aabb.getMax();

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

Intersection Triangle::intersect(const Ray& ray) const {
	glm::vec3 e1 = v3 - v2;
	glm::vec3 e2 = v1 - v3;
	glm::vec3 e3 = v2 - v1;
	glm::vec3 planeNormal = calculatePlaneNormal();
	if (!glm::epsilonEqual(glm::dot(ray.dir, planeNormal), 0.0f, 0.00001f)) {
		float d = glm::dot(v1, planeNormal);
		float t = (d - glm::dot(ray.origin, planeNormal)) / glm::dot(ray.dir, planeNormal);
		if (t < 0.00001f) {
			return Intersection();
		}
		glm::vec3 P = ray.origin + t * ray.dir;
		glm::vec3 d1 = P - v1;
		glm::vec3 d2 = P - v2;
		glm::vec3 d3 = P - v3;
		float totalArea = glm::dot(glm::cross(e1, e2), planeNormal) / 2.0f;
		bool b0 = (glm::dot(glm::cross(e1, d3), planeNormal) / 2.0f / totalArea) >= 0.0f;
		bool b1 = (glm::dot(glm::cross(e2, d1), planeNormal) / 2.0f / totalArea) >= 0.0f;
		bool b2 = (glm::dot(glm::cross(e3, d2), planeNormal) / 2.0f / totalArea) >= 0.0f;
		if (b0 && b1 && b2) {
			return Intersection(glm::distance(P, ray.origin), planeNormal);
		}
	}
	return Intersection();

	/*glm::vec3 planeNormal = calculatePlaneNormal();
	if (!glm::epsilonEqual(glm::dot(ray.dir, planeNormal), 0.0f, 0.01f)) {
		float d = glm::dot(v1, planeNormal);
		float t = d-glm::dot(planeNormal, ray.origin);
		if (t < 0.0f) {
			return Intersection();
		}
		glm::vec3 point = ray.origin + t * ray.dir;
		glm::vec3 perp;

		glm::vec3 e1 = v2 - v1;
		glm::vec3 d1 = point - v1;
		perp = glm::cross(e1, d1);
		if (glm::dot(planeNormal, perp) <= 0.0f) {
			return Intersection();
		}

		glm::vec3 e2 = v3 - v2;
		glm::vec3 d2 = point - v2;
		perp = glm::cross(e2, d2);
		if (glm::dot(planeNormal, perp) <= 0.0f) {
			return Intersection();
		}

		glm::vec3 e3 = v1 - v3;
		glm::vec3 d3 = point - v3;
		perp = glm::cross(e3, d3);
		if (glm::dot(planeNormal, perp) <= 0.0f) {
			return Intersection();
		}

		return Intersection(glm::distance(point, ray.origin), planeNormal);
	}
	return Intersection();*/
}

glm::vec3 Triangle::calculatePlaneNormal() const {
	return glm::cross(v2 - v1, v3 - v1);
	//return glm::cross(v3 - v2, v1 - v3);
}