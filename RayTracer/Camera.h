#pragma once
#include <glm/vec3.hpp>

class Camera
{
public:
	Camera(const glm::vec3& lookFrom, const glm::vec3& lookAt, const glm::vec3& up, float fovy);
	~Camera();

	struct Ray {
		glm::vec3 dir;
		glm::vec3 origin;
		Ray(const glm::vec3& org, const glm::vec3& dir) : origin(org), dir(dir) {}
	};

	 Ray createRay(float i, float j, int width, int height);

private:
	float fovy;
	glm::vec3 lookFrom;
	glm::vec3 lookAt;
	glm::vec3 up;
	glm::vec3 forward;
};

