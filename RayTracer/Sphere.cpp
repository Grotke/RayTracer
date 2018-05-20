#include "Sphere.h"
#include <algorithm>
#include "AABB.h"

Sphere::Sphere(const glm::vec3& center, float radius, const glm::mat4& transform, const Material& mat): Shape(transform, mat), center(center), radius(radius){

}

Sphere::~Sphere()
{
}

float Sphere::getMinX() const {
	float x = (transform * glm::vec4(center.x - radius, 0.0f, 0.0f, 1.0f)).x;
	return x;
}

float Sphere::getMinY() const {
	float y = (transform * glm::vec4(0.0f, center.y - radius, 0.0f, 1.0f)).y;
	return y;
}

float Sphere::getMinZ() const {
	float z = (transform * glm::vec4(0.0f, 0.0f, center.z -radius, 1.0f)).z;
	return z;
}

float Sphere::getMaxX() const {
	float x = (transform * glm::vec4(center.x + radius, 0.0f, 0.0f, 1.0f)).x;
	return x;
}

float Sphere::getMaxY() const {
	float y = (transform * glm::vec4(0.0f, center.y + radius, 0.0f, 1.0f)).y;
	return y;
}

float Sphere::getMaxZ() const {
	float z = (transform * glm::vec4(0.0f, 0.0f, center.z + radius, 1.0f)).z;
	return z;
}

bool Sphere::isInside(const AABB& box) const {
	glm::vec3 boxMin = glm::inverse(transform)*glm::vec4(box.getMin(), 1.0f);
	glm::vec3 boxMax = glm::inverse(transform)*glm::vec4(box.getMax(), 1.0f);
	float x = std::max(boxMin.x, std::min(center.x, boxMax.x));
	float y = std::max(boxMin.y, std::min(center.y, boxMax.y));
	float z = std::max(boxMin.z, std::min(center.z, boxMax.z));

	// this is the same as isPointInsideSphere
	float distance = std::sqrt((x - center.x) * (x - center.x) +
		(y - center.y) * (y - center.y) +
		(z - center.z) * (z - center.z));
	//box.contains(glm::normalize(box.getMidPoint() - center) * radius + center)
	//https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection says doesn't work when only colliding on face
	return box.contains(center) || distance < radius;
}

Intersection Sphere::intersect(const Ray& rawRay) const {
	Ray ray(glm::inverse(transform) * glm::vec4(rawRay.origin, 1.0f), glm::normalize(glm::inverse(transform) * glm::vec4(rawRay.dir, 0.0f)));
	glm::vec3 normalRayDir = glm::normalize(ray.dir);
	float a = glm::dot(normalRayDir, normalRayDir);
	float b = 2.0f * glm::dot(normalRayDir, (ray.origin - center));
	float c = glm::dot((ray.origin - center), (ray.origin - center)) - glm::pow(radius, 2.0f);
	float discrim = calculateDiscriminant(a, b, c);
	if (discrim < 0.0f) {
		return Intersection();
	}
	float x1 = (-b + glm::sqrt(discrim)) / 2.0f * a;
	float x2 = (-b - glm::sqrt(discrim)) / 2.0f * a;
	float t = std::min(x1, x2);
	if (t < 0.001f) {
		t = std::max(x1, x2);
		if (t < 0.001f) {
			return Intersection();
		}
	}
	glm::vec3 transfPoint = ray.origin + ray.dir * t;
	glm::vec3 normal = glm::transpose(glm::inverse(transform)) * glm::vec4(2.0f*(transfPoint - center), 0.0f);

	glm::vec3 finalPoint = transform * glm::vec4(transfPoint, 1.0f);
	return Intersection(glm::distance(finalPoint, rawRay.origin), normal);
}

float Sphere::calculateDiscriminant(float a, float b, float c) const {
	return glm::pow(b, 2) - (4 * a*c);
}