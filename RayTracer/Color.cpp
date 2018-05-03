#include "Color.h"

Color::Color(float inR, float inG, float inB) : r(inR), g(inG), b(inB) {
}

Color::Color() : r(0.0f), g(0.0f), b(0.0f) {
}

Color::~Color()
{
}

int Color::getR() const {
	return glm::clamp((int)(r * 255), min, max);
}

int Color::getG() const {
	return glm::clamp((int)(g * 255), min, max);
}

int Color::getB() const {
	return glm::clamp((int)(b * 255), min, max);
}

Color Color::operator+(const Color& other) const
{
	return Color(r + other.r, g + other.g, b + other.b);
}

Color& Color::operator+=(const Color& color2) {
	this->r += color2.r;
	this->g += color2.g;
	this->b += color2.b;
	return *this;
}

glm::vec3 Color::getAsFloat() const {
	return glm::vec3(r / (float)max, g / (float)max, b / (float)max);
}

Color Color::operator*(float x) const {
	return Color(r * x, g * x, b * x);
}

Color Color::operator*(const Color& other) const {
	return Color(r * other.r, g * other.g, b * other.b);
}