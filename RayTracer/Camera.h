#pragma once
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <vector>
#include "SceneObjects.hpp"

class Camera
{
public:
	Camera(const glm::vec3& lookFrom, const glm::vec3& lookAt, const glm::vec3& up, float fovy);
	Camera();
	~Camera();
	 Ray createRayToPixel(float i, float j, int width, int height) const;
	 const glm::vec3& getEye() const;
	 static glm::vec3 createPointFromRay(const Ray& ray, float t);

private:
	float fovy;
	glm::vec3 lookFrom;
	glm::vec3 lookAt;
	glm::vec3 up;
	glm::vec3 forward;
	friend std::ostream& operator<<(std::ostream &strm, const Camera &cam);
};

