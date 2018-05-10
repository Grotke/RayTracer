#include "Partition.h"
#include <algorithm>



Partition::Partition(const std::vector<Shape*>& objects)
{
	AABB box;
	for (Shape* obj : objects) {
		box.expand(*obj);
	}
	splitThreshold = std::min(splitThreshold, std::max((int)(0.1f * objects.size()), 1));
	root = new PartitionNode(box, objects.size());
	for (Shape* obj : objects) {
		insert(obj, root);
	}
	split(root);
}


Partition::~Partition()
{
}

Intersection Partition::findClosestintersection(const Ray& ray) const {
	return intersect(ray, root);
}

Intersection Partition::intersect(const Ray& ray, const PartitionNode const * currentNode) const {
	if (currentNode == nullptr || !currentNode->box.intersect(ray).isValidIntersection()) {
		return Intersection();
	}
	if (currentNode->isLeaf()) {
		return currentNode->findClosestIntersection(ray);
	}
	Intersection left = intersect(ray, currentNode->left);
	Intersection right = intersect(ray, currentNode->right);
	//If neither intersection is valid return no intersection
	if (!(left.isValidIntersection() || right.isValidIntersection())) {
		return Intersection();
	}
	if ((left.isValidIntersection() && !right.isValidIntersection()) || left.distAlongRay <= right.distAlongRay) {
		return left;
	}
	if ((right.isValidIntersection() && !left.isValidIntersection()) || right.distAlongRay < left.distAlongRay) {
		return right;
	}
}

void Partition::insert(Shape* obj, PartitionNode* nodeToInsert) {
	if (nodeToInsert->isLeaf() && nodeToInsert->box.contains(*obj)) {
		nodeToInsert->objects.push_back(obj);
		//if the node pushes the partition above a threshold (50% of objects parents are in this node)
		//What happens if only one object? ie Quickly reach threshold and no way to go below it?
	}
	else {
		//Object might be in both left and right so checking here.
		if (nodeToInsert->left != nullptr && nodeToInsert->left->box.contains(*obj)) {
			insert(obj, nodeToInsert->left);
		}
		if (nodeToInsert->right != nullptr && nodeToInsert->right->box.contains(*obj)) {
			insert(obj, nodeToInsert->right);
		}
	}
}

void Partition::insert(Shape*obj) {
	insert(obj, root);
}

void Partition::split(PartitionNode* nodeToSplit) {
	if (nodeToSplit != nullptr && nodeToSplit->isLeaf() && nodeToSplit->objects.size() > splitThreshold) {
		nodeToSplit->left = new PartitionNode(nodeToSplit->box.splitLeft(), nodeToSplit->objects.size());
		nodeToSplit->right = new PartitionNode(nodeToSplit->box.splitRight(), nodeToSplit->objects.size());
		for (Shape* obj : nodeToSplit->objects) {
			insert(obj, nodeToSplit->left);
			insert(obj, nodeToSplit->right);
		}

		if (nodeToSplit->left->isEmpty() && !nodeToSplit->right->isEmpty()) {
			delete nodeToSplit->left;
			nodeToSplit->left = nullptr;
		}
		else if (nodeToSplit->right->isEmpty() && !nodeToSplit->left->isEmpty()) {
			delete nodeToSplit->right;
			nodeToSplit->right = nullptr;
		}
		split(nodeToSplit->left);
		split(nodeToSplit->right);
	}
}
