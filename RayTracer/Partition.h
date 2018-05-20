#pragma once
#include <vector>
#include "SceneObjects.hpp"
#include "Shape.h"
#include "AABB.h"

class Partition{
	public:
		Partition(const std::vector<Shape*>& objects);
		~Partition();
		void insert(Shape* object);
		Intersection findClosestintersection(const Ray& ray) const;

	private:
		float splitThreshold = 0.5f;
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

				for (Shape* obj: objects) {
					Intersection currentIntersect = obj->intersect(ray);
					if (currentIntersect.isValidIntersection() && (currentIntersect.distAlongRay < objIntersect.distAlongRay || !objIntersect.isValidIntersection())) {
						objIntersect = currentIntersect;
						objIntersect.mat = obj->getMaterial();
					}
				}
				return objIntersect;
			}
			PartitionNode(const glm::vec3& minBound, const glm::vec3& maxBound, int parentObjectCount) : box(minBound, maxBound), parentObjectCount(parentObjectCount) {
			}
			PartitionNode(const AABB& box, int parentObjectCount) : box(box), parentObjectCount(parentObjectCount) {}
		};
		void split(PartitionNode* nodeToSplit, int prevMatches);
		bool insert(Shape* object, PartitionNode* nodeToInsert);
		Intersection intersect(const Ray& ray, PartitionNode const * const currentNode) const;
		PartitionNode* root;
		void deallocateTree(PartitionNode*& node);
};

