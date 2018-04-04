#pragma once
#include <glm/glm.hpp>
class Transform {
public:
	static glm::mat4 translate(float x, float y, float z) {
		return glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, x, y, z, 1.0f);
	}

	static glm::mat4 scale(float x, float y, float z) {
		return glm::mat4(x, 0.0f, 0.0f, 0.0f, 0.0f, y, 0.0f, 0.0f, 0.0f, 0.0f, z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	}

	static glm::mat4 rotate(float x, float y, float z, float angleInDegrees) {
		float angleInRadians = glm::radians(angleInDegrees);
		float cosA = glm::cos(angleInRadians);
		float sinA = glm::sin(angleInRadians);
		return glm::mat4(cosA + (1.0f - cosA)*x*x, (1.0f - cosA)*x*y + sinA * z, (1.0f - cosA)*x*z - sinA * y, 0.0f, (1.0f - cosA)*x*y - sinA * z, cosA + (1.0f - cosA)*y*y, (1.0f - cosA)*y*z + sinA * x, 0.0f, (1.0f - cosA)*x*z + sinA * y, (1.0f - cosA)*y*z - sinA * x, cosA + (1.0f - cosA)*z*z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	}
};