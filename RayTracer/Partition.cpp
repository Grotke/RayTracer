#include "Partition.h"
#include <algorithm>
#include <iostream>



Partition::Partition(const std::vector<Shape*>& objects)
{
	AABB box;
	for (Shape* obj : objects) {
		box.expand(*obj);
	}
	//splitThreshold = std::min(splitThreshold, std::max((int)(0.1f * objects.size()), 1));
	root = new PartitionNode(box, objects.size());
	for (Shape* obj : objects) {
		insert(obj, root);
	}
	split(root, 0);
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
	else if (currentNode->isLeaf()) {
		//std::cout << currentNode->objects.size() << std::endl;
		return currentNode->findClosestIntersection(ray);
	}
	Intersection left = intersect(ray, currentNode->left);
	Intersection right = intersect(ray, currentNode->right);
	//If neither intersection is valid return no intersection
	if (!(left.isValidIntersection() || right.isValidIntersection())) {
		return Intersection();
	}
	else if ((left.isValidIntersection() && !right.isValidIntersection()) || left.distAlongRay <= right.distAlongRay) {
		return left;
	}
	else{
		return right;
	}
}

bool Partition::insert(Shape* obj, PartitionNode* nodeToInsert) {
	if (nodeToInsert->isLeaf() && nodeToInsert->box.contains(*obj)) {
		nodeToInsert->objects.push_back(obj);
		return true;
		//if the node pushes the partition above a threshold (50% of objects parents are in this node)
		//What happens if only one object? ie Quickly reach threshold and no way to go below it?
	}
	else {
		bool inLeft = false, inRight = false;
		//Object might be in both left and right so checking here.
		if (nodeToInsert->left != nullptr && nodeToInsert->left->box.contains(*obj)) {
			inLeft = insert(obj, nodeToInsert->left);
		}
		if (nodeToInsert->right != nullptr && nodeToInsert->right->box.contains(*obj)) {
			inRight = insert(obj, nodeToInsert->right);
		}
		return inLeft || inRight;
	} 
}

void Partition::insert(Shape*obj) {
	insert(obj, root);
}

void Partition::split(PartitionNode* nodeToSplit, int prevMatches) {
	if (nodeToSplit != nullptr && nodeToSplit->isLeaf() && ((float)prevMatches/nodeToSplit->objects.size()) < splitThreshold) {
		//std::cout << "Just Split" << std::endl;
		nodeToSplit->left = new PartitionNode(nodeToSplit->box.splitLeft(), nodeToSplit->objects.size());
		nodeToSplit->right = new PartitionNode(nodeToSplit->box.splitRight(), nodeToSplit->objects.size());
		int matches = 0;
		for (Shape* obj : nodeToSplit->objects) {
			bool inLeft = insert(obj, nodeToSplit->left);
			bool inRight = insert(obj, nodeToSplit->right);
			if (inLeft && inRight) {
				matches++;
			}
		}

		if (nodeToSplit->left->isEmpty() && !nodeToSplit->right->isEmpty()) {
			delete nodeToSplit->left;
			nodeToSplit->left = nullptr;
			//std::cout << "Left half is empty" << std::endl;
		}
		else if (nodeToSplit->right->isEmpty() && !nodeToSplit->left->isEmpty()) {
			delete nodeToSplit->right;
			nodeToSplit->right = nullptr;
			//std::cout << "Right half is empty" << std::endl;
		}
		split(nodeToSplit->left, matches);
		split(nodeToSplit->right, matches);
	}
}
