#include "AABB.h"
#include <glm/gtc/epsilon.hpp>
#include <algorithm>



AABB::AABB(const glm::vec3& min, const glm::vec3& max): min(min), max(max)
{
}

AABB::AABB() {

}


AABB::~AABB()
{
}

float AABB::getMinX() const {
	return min.x;
}

float AABB::getMinY() const {
	return min.y;
}

float AABB::getMinZ() const {
	return min.z;
}

float AABB::getMaxX() const {
	return max.x;
}

float AABB::getMaxY() const {
	return max.y;
}

float AABB::getMaxZ() const {
	return max.z;
}

Intersection AABB::intersect(const Ray& ray) const {
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
		return Intersection();
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax) {
		return Intersection();
	}
	float t_result = tmin;

	// If tmin is < 0, tmax is closer
	if (tmin < 0.0f) {
		t_result = tmax;
	}

	glm::vec3 normals[] = {
		glm::vec3(-1, 0, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(0, -1, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 0, -1),
		glm::vec3(0, 0, 1)
	};
	float t[] = { t1, t2, t3, t4, t5, t6 };
	glm::vec3 normal;
	for (int i = 0; i < 6; ++i) {
		if(glm::epsilonEqual(t_result, t[i], 0.0001f)){
			normal = normals[i];
		}
	}
	return Intersection(t_result, normal);
}

bool AABB::contains(const glm::vec3& point) const {
	return point.x <= max.x && point.x >= min.x && point.y <= max.y && point.y >= min.y && point.z <= max.z && point.z >= min.z;
}

AABB::Axis AABB::getLongestAxis() const {
	float maxAxis = std::max(max.z - min.z, std::max(max.x-min.x, max.y-min.y));
	if (max.z - min.z == maxAxis) {
		return Axis::Z;
	}
	else if (max.y - min.y == maxAxis) {
		return Axis::Y;
	}
	return Axis::X;
}

glm::vec3 AABB::getMidPoint() const {
	return max - min / 2.0f;
}

AABB AABB::splitLeft() const {
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

AABB AABB::splitRight() const {
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

void AABB::expand(const Shape& obj) {
	if (obj.getMinX() < min.x) { min.x = obj.getMinX(); }
	if (obj.getMinY() < min.y) { min.y = obj.getMinY(); }
	if (obj.getMinZ() < min.z) { min.z = obj.getMinZ(); }
	if (obj.getMaxX() > max.x) { max.x = obj.getMaxX(); }
	if (obj.getMaxY() > max.y) { max.y = obj.getMaxY(); }
	if (obj.getMaxZ() > max.z) { max.z = obj.getMaxZ(); }
}