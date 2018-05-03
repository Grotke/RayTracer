#include "Color.h"



Color::Color()
{
}


Color::~Color()
{
}

float r, g, b;
int min = 0, max = 255;
Color(float inR, float inG, float inB) : r(inR), g(inG), b(inB) {
}

Color() : r(0.0f), g(0.0f), b(0.0f) {}

int getR() {
	return glm::clamp((int)(r * 255), min, max);
}

int getG() {
	return glm::clamp((int)(g * 255), min, max);
}

int getB() {
	return glm::clamp((int)(b * 255), min, max);
}

Color operator+(const Color& other) const
{
	return Color(r + other.r, g + other.g, b + other.b);
}

Color& operator+=(const Color& color2) {
	this->r += color2.r;
	this->g += color2.g;
	this->b += color2.b;
	return *this;
}

glm::vec3 getAsFloat() const {
	return glm::vec3(r / (float)max, g / (float)max, b / (float)max);
}