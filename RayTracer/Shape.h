#pragma once
#include <glm/glm.hpp>
#include "SceneObjects.hpp"
class AABB;
class Shape
{
public:
	Shape(const glm::mat4& transform, const Material& mat);
	Shape();
	virtual ~Shape();
	virtual float getMinX() const;
	virtual float getMinY() const;
	virtual float getMinZ() const;
	virtual float getMaxX() const;
	virtual float getMaxY() const;
	virtual float getMaxZ() const;
	virtual bool isInside(const AABB& box) const;
	virtual Intersection intersect(const Ray& ray) const;
	const Material& getMaterial() const;

protected:
	glm::mat4 transform;
	const Material material;
};

