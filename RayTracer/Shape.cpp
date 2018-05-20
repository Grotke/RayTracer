#include "Shape.h"
#include "AABB.h"

Shape::Shape(const glm::mat4& transform, const Material& mat): transform(transform), material(mat)
{
}

Shape::Shape() {

}

Shape::~Shape()
{
}

float Shape::getMinX() const {
	return 0.0f;
}

float Shape::getMinY() const {
	return 0.0f;
}

float Shape::getMinZ() const {
	return 0.0f;
}

float Shape::getMaxX() const {
	return 0.0f;
}

float Shape::getMaxY() const {
	return 0.0f;
}

float Shape::getMaxZ() const {
	return 0.0f;
}

bool Shape::isInside(const AABB& box) const {
	return false;
}

Intersection Shape::intersect(const Ray& ray) const {
	return Intersection();
}

const Material& Shape::getMaterial() const {
	return material;
}
