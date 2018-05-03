#pragma once
#include "Shape.h"
class Sphere :
	public Shape
{
public:
	Sphere(const glm::vec3& center, float radius, const glm::mat4& transform, const Material& mat);
	virtual ~Sphere();
	virtual float getMinX() const override;
	virtual float getMinY() const override;
	virtual float getMinZ() const override;
	virtual float getMaxX() const override;
	virtual float getMaxY() const override;
	virtual float getMaxZ() const override;
	virtual bool isInside(const AABB& box) const override;
	virtual Intersection intersect(const Ray& ray) const override;

private:
	const glm::vec3 center;
	float radius;
	float calculateDiscriminant(float a, float b, float c) const;
};

