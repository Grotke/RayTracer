#include "Camera.h"
#include <glm/trigonometric.hpp>

Camera::Camera(const glm::vec3& lookFrom, const glm::vec3& lookAt, const glm::vec3& up, float fovy) : lookFrom(lookFrom), lookAt(lookAt), up(glm::normalize(up)), fovy(fovy)
{
	forward = lookAt - lookFrom;
}

Camera::Camera(){}

Camera::~Camera()
{
}

Ray Camera::createRayToPixel(float i, float j, int width, int height) const {
	glm::vec3 w = -glm::normalize(forward);
	glm::vec3 u = glm::normalize(glm::cross(up, w));
	glm::vec3 v = glm::normalize(glm::cross(w, u));
	float alpha = glm::tan(glm::radians(fovy / 2.0f)) * (static_cast<float>(width)/height) * ((i - (width/2.0f))/(width/2.0f));
	float beta = glm::tan(glm::radians(fovy / 2.0f)) * (((height / 2.0f) - j) / (height / 2.0f));
	glm::vec3 rayDir = (alpha * u) + (beta * v) - w;

	return Ray(lookFrom, rayDir);
}

glm::vec3 Camera::createPointFromRay(const Ray& ray, float t) {
	return ray.origin + glm::normalize(ray.dir)*t;
}

const glm::vec3& Camera::getEye() const {
	return lookFrom;
}


