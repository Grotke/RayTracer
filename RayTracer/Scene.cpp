#include <fstream>
#include <iostream>
#include <sstream>
#include <glm/glm.hpp>
#include <stack>
#include "Scene.h"
#include "Transform.h"

void Scene::setDefaults() {
	attenuation = glm::vec3(1.0f, 0.0f, 0.0f);
	maxDepth = 5;
	outputFileName = "test.png";
}

Scene::Scene(const std::string& fileName){
	Color diffuse = Color(0.0f, 0.0f, 0.0f), specular = Color(0.0f, 0.0f,0.0f), emission = Color(0.0f, 0.0f, 0.0f), ambient = Color(0.2f, 0.2f, 0.2f);
	int numObjects = 0, maxObjects = 200;
	int numVerts, numVertNorms;
	glm::vec3 * verts = NULL;
	glm::vec3 * vertNorms = NULL;
	int vertIndex = 0;
	int vertNormIndex = 0;
	float shininess = 0;
	setDefaults();
	std::string line, cmd;
	int numUsed = 0, numLights = 100;
	std::ifstream inFile;
	inFile.open(fileName);
	if (inFile.is_open()) {
		std::stack<glm::mat4> transfstack;
		transfstack.push(glm::mat4(1.0));
		std::getline(inFile, line);
		while (inFile) {
			if ((line.find_first_not_of(" \t\r\n") != std::string::npos) && (line[0] != '#')) {
				std::stringstream s(line);
				s >> cmd;
				float values[10];
				bool isValidInput;
				if (cmd == "directional") {
						isValidInput = readvals(s, 6, values);
						if (isValidInput) {
							lights.push_back(Light(glm::vec4(values[0], values[1], values[2], 0.0f), Color(values[3], values[4], values[5])));
							numDirectionalLights++;
						}
				}
				else if (cmd == "point") {
						isValidInput = readvals(s, 6, values);
						if (isValidInput) {
							lights.push_back(Light(glm::vec4(values[0], values[1], values[2], 1.0f), Color(values[3], values[4], values[5])));
							numPointLights++;
						}
				}
				else if (cmd == "attenuation") {
					isValidInput = readvals(s, 3, values);
					if (isValidInput) {
						attenuation = glm::vec3(values[0], values[1], values[2]);
					}
				}

				else if (cmd == "maxverts") {
					isValidInput = readvals(s, 1, values);
					if (isValidInput) {
						numVerts = (int)values[0];
						verts = new glm::vec3[numVerts];
					}
				}

				else if (cmd == "maxvertnorms") {
					isValidInput = readvals(s, 1, values);
					if (isValidInput) {
						numVertNorms = (int)values[0]*2;
						vertNorms = new glm::vec3[numVertNorms];
					}
				}
				else if (cmd == "vertex") {
					isValidInput = readvals(s, 3, values);
					if (isValidInput) {
						verts[vertIndex] = glm::vec3(values[0], values[1], values[2]);
						vertIndex++;
					}
				}
				else if (cmd == "vertexnormal") {
					isValidInput = readvals(s, 6, values);
					if (isValidInput) {
						vertNorms[vertNormIndex] = glm::vec3(values[0], values[1], values[2]);
						vertNormIndex++;
						vertNorms[vertNormIndex] = glm::vec3(values[3], values[4], values[5]);
						vertNormIndex++;
					}
				}
				else if (cmd == "ambient") {
					isValidInput = readvals(s, 3, values);
					if (isValidInput) {
						ambient = Color((float)values[0], (float)values[1], (float)values[2]);
					}
				}
				else if (cmd == "diffuse") {
					isValidInput = readvals(s, 3, values);
					if (isValidInput) {
						diffuse = Color(values[0], values[1], values[2]);
					}
				}
				else if (cmd == "specular") {
					isValidInput = readvals(s, 3, values);
					if (isValidInput) {
						specular = Color(values[0], values[1], values[2]);
					}
				}
				else if (cmd == "emission") {
					isValidInput = readvals(s, 3, values);
					if (isValidInput) {
						emission = Color(values[0], values[1], values[2]);
					}
				}
				else if (cmd == "shininess") {
					isValidInput = readvals(s, 1, values);
					if (isValidInput) {
						shininess = values[0];
					}
				}
				else if (cmd == "size") {
					isValidInput = readvals(s, 2, values);
					if (isValidInput) {
						width = (int)values[0]; 
						height = (int)values[1];
					}
				}
				else if (cmd == "maxdepth") {
					isValidInput = readvals(s, 1, values);
					if (isValidInput) {
						maxDepth = (int)values[0];
					}
				}
				else if (cmd == "output") {
					s >> outputFileName;
					if (s.fail()) {
						std::cout << "Failed reading value will skip\n";
					}
				}
				else if (cmd == "camera") {
					isValidInput = readvals(s, 10, values);
					if (isValidInput) {
						cam = Camera(glm::vec3(values[0], values[1], values[2]), glm::vec3(values[3], values[4], values[5]), glm::vec3(values[6], values[7], values[8]), values[9]);
					}
				}
				else if (cmd == "sphere" || cmd == "tri" || cmd == "trinormal") {
							Material mat(diffuse, specular, emission, ambient, shininess);
							Shape* obj;

							if (cmd == "sphere") {
								isValidInput = readvals(s, 4, values);
								if (isValidInput) {
									obj = new Shape(glm::vec3(values[0], values[1], values[2]), values[3], mat);
									obj->transform = transfstack.top();
									objects.push_back(obj);
									numSpheres++;
								}
							}
							else if (cmd == "tri") {
								isValidInput = readvals(s, 3, values);
								if (isValidInput) {
									obj = new Shape(verts[(int)values[0]], verts[(int)values[1]], verts[(int)values[2]], mat);
									obj->transform = transfstack.top();
									obj->transformVerts();
									objects.push_back(obj);
									numTriangles++;
								}
							}
							else if (cmd == "trinormal") {
								isValidInput = readvals(s, 6, values);
								if (isValidInput) {
									obj = new Shape(vertNorms[(int)values[0]], vertNorms[(int)values[2]], vertNorms[(int)values[4]], mat);
									obj->n1 = vertNorms[(int)values[1]];
									obj->n2 = vertNorms[(int)values[3]];
									obj->n3 = vertNorms[(int)values[5]];
									obj->transform = transfstack.top();
									obj->transformVerts();
									obj->transformNorms();
									objects.push_back(obj);
									numTriangles++;
								}
							}
				}
				else if (cmd == "translate") {
					isValidInput = readvals(s, 3, values);
					if (isValidInput) {
						transfstack.top() = transfstack.top()*Transform::translate(values[0], values[1], values[2]);
					}
				}
				else if (cmd == "scale") {
					isValidInput = readvals(s, 3, values);
					if (isValidInput) {
						transfstack.top() = transfstack.top()*Transform::scale(values[0], values[1], values[2]);
					}
				}
				else if (cmd == "rotate") {
					isValidInput = readvals(s, 4, values);
					if (isValidInput) {
						transfstack.top() = transfstack.top() * Transform::rotate(values[0], values[1], values[2], values[3]);
					}
				}
				else if (cmd == "pushTransform") {
					transfstack.push(transfstack.top());
				}
				else if (cmd == "popTransform") {
					if (transfstack.size() <= 1) {
						std::cerr << "Stack has no elements.  Cannot Pop\n";
					}
					else {
						transfstack.pop();
					}
				}
				else {
					std::cerr << "Unknown Command: " << cmd << " Skipping \n";
				}
			}
				std::getline(inFile, line);
			}
			isLoaded = true;
	}else{
		isLoaded = false;
		std::cout << "Unable to open file " << fileName << std::endl;
	}
}


Scene::~Scene()
{
}

bool Scene::readvals(std::stringstream &s, int numvals, float* values)
{
	for (int i = 0; i < numvals; i++) {
		s >> values[i];
		if (s.fail()) {
			std::cout << "Failed reading value " << i << " will skip\n";
			return false;
		}
	}
	return true;
}
  

const Camera& Scene::getCamera() const {
	return cam;
}

const std::vector<Shape*>& Scene::getSceneObjects() const {
	return objects;
}

const std::string& Scene::getOutputFileName() const {
	return outputFileName;
}

const unsigned int Scene::getWidth() const {
	return width;
}

const unsigned int Scene::getHeight() const {
	return height;
}

const std::vector<Light>& Scene::getLights() const {
	return lights;
}

int Scene::getNumObjects() const {
	return objects.size();
}

int Scene::getNumSpheres() const {
	return numSpheres;
}

int Scene::getNumTriangles() const {
	return numTriangles;
}

int Scene::getNumLights() const {
	return numPointLights + numDirectionalLights;
}

int Scene::getNumDirectionalLights() const {
	return numDirectionalLights;
}

int Scene::getNumPointLights() const {
	return numPointLights;
}

bool Scene::loaded() const {
	return isLoaded;
}
