#pragma once
#include "Shape.h"

class Triangle :
	public Shape
{
	public:
		Triangle(const glm::vec3& inV1, const glm::vec3& inV2, const glm::vec3& inV3, const glm::mat4& inTransform, const Material& mat, const glm::vec3& inN1, const glm::vec3& inN2, const glm::vec3& inN3);
		Triangle(const glm::vec3& inV1, const glm::vec3& inV2, const glm::vec3& inV3, const glm::mat4& inTransform, const Material& mat);
		virtual ~Triangle();
		virtual float getMinX() const override;
		virtual float getMinY() const override;
		virtual float getMinZ() const override;
		virtual float getMaxX() const override;
		virtual float getMaxY() const override;
		virtual float getMaxZ() const override;
		virtual bool isInside(const AABB& box) const override;
		virtual Intersection intersect(const Ray& ray) const override;
		glm::vec3 v1, v2, v3;
		glm::vec3 n1, n2, n3;

	private:
		bool OverlapOnAxis(const AABB& aabb, const glm::vec3& axis) const;
		glm::vec2 GetInterval(const AABB& aabb, const glm::vec3& axis) const;
		glm::vec2 GetInterval(const glm::vec3& axis) const;
		glm::vec3 calculatePlaneNormal() const;
};

