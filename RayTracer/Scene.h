#pragma once
#include <string>
#include <sstream>
#include <vector>

#include "SceneObjects.hpp"
#include "Camera.h"
#include "Partition.h"
#include "Shape.h"
#include "Sphere.h"
#include "Triangle.h"

class Scene
{
	public:
		Scene(const std::string& fileName);
		~Scene();
		bool readvals(std::stringstream &s, int numvals, float * values);
		void setDefaults();

		const Camera& getCamera() const;
		const std::vector<Shape *>& getSceneObjects() const;
		const std::string& getOutputFileName() const;
		const unsigned int getWidth() const;
		const unsigned int getHeight() const;
		const std::vector<Light>& getLights() const;
		int getNumObjects() const;
		int getNumSpheres() const;
		int getNumTriangles() const;
		int getNumLights() const;
		int getNumDirectionalLights() const;
		int getNumPointLights() const;
		bool loaded() const;
		glm::vec3 attenuation;
		int maxDepth;
		Color backgroundColor;
		Intersection findClosestIntersection(const Ray& ray) const;

	private:
		Partition* objectTree;
		Camera cam;
		bool isLoaded;
		std::vector<Light> lights;
		unsigned int width, height;
		std::string outputFileName;
		std::vector<Shape*> objects;
		int numTriangles, numPointLights, numDirectionalLights, numSpheres;
};

