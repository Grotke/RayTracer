#include "Partition.h"
#include <algorithm>

Partition::Partition(const std::vector<Shape*>& objects){
	AABB box;
	for (Shape* obj : objects) {
		box.expand(*obj);
	}
	root = new PartitionNode(box, objects.size());
	for (Shape* obj : objects) {
		insert(obj, root);
	}
	split(root, 0);
}

Partition::~Partition(){
	deallocateTree(root);
}

void Partition::deallocateTree(PartitionNode*& node) {
	if (node != nullptr) {
		if (!node->isLeaf()) {
			deallocateTree(node->left);
			deallocateTree(node->right);
		}
		delete node;
		node = nullptr;
	}
}

Intersection Partition::findClosestintersection(const Ray& ray) const {
	return intersect(ray, root);
}

Intersection Partition::intersect(const Ray& ray, PartitionNode const * const currentNode) const {
	if (currentNode == nullptr || !currentNode->box.intersect(ray).isValidIntersection()) {
		return Intersection();
	}
	else if (currentNode->isLeaf()) {
		return currentNode->findClosestIntersection(ray);
	}
	Intersection left = intersect(ray, currentNode->left);
	Intersection right = intersect(ray, currentNode->right);
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
	if (nodeToSplit != nullptr && nodeToSplit->isLeaf() && (std::static_cast<float>(prevMatches)/nodeToSplit->objects.size()) < splitThreshold) {
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
		}
		else if (nodeToSplit->right->isEmpty() && !nodeToSplit->left->isEmpty()) {
			delete nodeToSplit->right;
			nodeToSplit->right = nullptr;
		}
		split(nodeToSplit->left, matches);
		split(nodeToSplit->right, matches);
	}
}
