#include "Sphere.h"
#include <algorithm>
#include "AABB.h"



Sphere::Sphere(const glm::vec3& center, float radius, const glm::mat4& transform, const Material& mat): Shape(transform, mat), center(center), radius(radius){

}


Sphere::~Sphere()
{
}

float Sphere::getMinX() const {
	return center.x - radius;
}
float Sphere::getMinY() const {
	return center.y - radius;
}
float Sphere::getMinZ() const {
	return center.z - radius;
}
float Sphere::getMaxX() const {
	return center.x + radius;
}

float Sphere::getMaxY() const {
	return center.y + radius;
}

float Sphere::getMaxZ() const {
	return center.z + radius;
}

bool Sphere::isInside(const AABB& box) const {
	glm::vec3 boxMin = box.getMin();
	glm::vec3 boxMax = box.getMax();
	float x = std::max(boxMin.x, std::min(center.x, boxMax.x));
	float y = std::max(boxMin.y, std::min(center.y, boxMax.y));
	float z = std::max(boxMin.z, std::min(center.z, boxMax.z));

	// this is the same as isPointInsideSphere
	float distance = std::sqrt((x - center.x) * (x - center.x) +
		(y - center.y) * (y - center.y) +
		(z - center.z) * (z - center.z));

	//https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection says doesn't work when only colliding on face
	return box.contains(center) || box.contains(glm::normalize(box.getMidPoint() - center) * radius + center) || radius > glm::length(box.getMidPoint() - center) || distance < radius;
}

Intersection Sphere::intersect(const Ray& rawRay) const {
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

float Sphere::calculateDiscriminant(float a, float b, float c) const {
	return glm::pow(b, 2) - (4 * a*c);
}