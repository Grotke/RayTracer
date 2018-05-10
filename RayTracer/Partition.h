#pragma once
#include <vector>
#include "SceneObjects.hpp"
#include "Camera.h"
#include "Shape.h"
#include "AABB.h"

class Partition
{
public:
	Partition(const std::vector<Shape*>& objects);
	~Partition();
	void insert(Shape* object);
	Intersection findClosestintersection(const Ray& ray) const;

private:
	int splitThreshold = 5;
	struct PartitionNode {
		int parentObjectCount;
		AABB box;
		PartitionNode* left = nullptr;
		PartitionNode* right = nullptr;
		std::vector<Shape *> objects;
		bool isEmpty() const {
			return objects.size() == 0;
		}
		bool isLeaf() const {
			return left == nullptr && right == nullptr;
		}
		Intersection findClosestIntersection(const Ray& ray) const {
			Intersection objIntersect;

			for (int i = 0; i < objects.size(); i++) {
				//I don't know why, but when I had a const reference to Ray it wouldn't let me do a *this so I changed it to use pointers instead
				Intersection currentIntersect = objects[i]->intersect(ray);
				if (currentIntersect.isValidIntersection() && (currentIntersect.distAlongRay < objIntersect.distAlongRay || !objIntersect.isValidIntersection())) {
					objIntersect = currentIntersect;
					objIntersect.objectIndex = i;
				}
			}
			return objIntersect;
		}
		PartitionNode(const glm::vec3& minBound, const glm::vec3& maxBound, int parentObjectCount) : box(minBound, maxBound), parentObjectCount(parentObjectCount) {
		}
		PartitionNode(const AABB& box, int parentObjectCount) : box(box), parentObjectCount(parentObjectCount) {}
	};
	void split(PartitionNode* nodeToSplit);
	void insert(Shape* object, PartitionNode* nodeToInsert);
	Intersection intersect(const Ray& ray, const PartitionNode const * currentNode) const;
	PartitionNode* root;
};

