#pragma once
#include <glm/glm.hpp>
class Color
{
	public:
		Color();
		~Color();
		Color(float inR, float inG, float inB);
		Color operator+(const Color& other) const;
		int getR() const;
		int getG() const;
		int getB() const;
		Color& operator+=(const Color& other);
		glm::vec3 getAsFloat() const;
		Color operator*(float x) const;
		Color operator*(const Color& other) const;

	private:
		float r, g, b;
		int min = 0, max = 255;
};

