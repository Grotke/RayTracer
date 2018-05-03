#pragma once
#include "Shape.h"
class AABB :
	public Shape
{
public:
	AABB(const glm::vec3& min, const glm::vec3& max);
	AABB();
	virtual ~AABB();
	virtual float getMinX() const override;

	virtual float getMinY() const override;

	virtual float getMinZ() const override;

	virtual float getMaxX() const override;

	virtual float getMaxY() const override;

	virtual float getMaxZ() const override;
	glm::vec3 getMin() const;
	glm::vec3 getMax() const;
	virtual Intersection intersect(const Ray& ray) const override;
	bool contains(const Shape& obj) const;
	bool contains(const glm::vec3& point) const;
	glm::vec3 getMidPoint() const;
	AABB splitLeft() const;
	AABB splitRight() const;
	void expand(const Shape& obj);
private:
	enum class Axis {
		X,
		Y,
		Z
	};

	glm::vec3 min = glm::vec3(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
	glm::vec3 max = glm::vec3(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
	Axis getLongestAxis() const;

};

