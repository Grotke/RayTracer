#include <iostream>
#include <algorithm>
#include <vector>

#include "FreeImage.h"
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

#include "Renderer.h"
#include "Camera.h"


/*
	Problems Encountered
		Fovx calculation
			Fovx was being calculated incorrectly because the width and height were doing an integer division, 
			dropping the fraction of the division so instead of multiplying by 1.33 it was multiplying by 1, making fovx and fovy the same. 
			Once converted one to a float, the fovx was calculated correctly.
		Color order
			FreeImage reads the colors differently depending on the endianness of the system. My system is apparently little? endian and FreeImage
			is expecting a big? endian system (or vice versa). So instead of entering colors as rgb I go by bgr.
*/
struct Color {
	int r, g, b;
	Color(int r, int g, int b): r(r), g(g), b(b) {}
	Color() {}
};

struct Shape {
	bool isTriangle;
	const glm::vec3 v1, v2, v3;
	const glm::vec3 center;
	float radius;
	const Color color;
	Shape(glm::vec3 center, float radius, const Color& color) : center(center), radius(radius), isTriangle(false), color(color) {}
	Shape(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const Color& color) : v1(v1), v2(v2), v3(v3), isTriangle(true), color(color) {}
};

float calculateDiscriminant(float a, float b, float c) {
	return glm::pow(b, 2) - (4 * a*c);
}

float intersectTriangle(const Camera::Ray& ray, const Shape& tri) {
	glm::vec3 planeNormal = glm::normalize(glm::cross(tri.v2 - tri.v1, tri.v3 - tri.v1));
	if (!glm::epsilonEqual(glm::dot(ray.dir, planeNormal), 0.0f, 0.1f)) {
		float distAlongRay = (glm::dot(tri.v1, planeNormal) - glm::dot(ray.origin, planeNormal)) / glm::dot(ray.dir, planeNormal);
		glm::vec3 P = ray.origin + ray.dir*distAlongRay;
		glm::vec3 A = tri.v1;
		glm::vec3 C = tri.v2;
		glm::vec3 B = tri.v3;
		bool b0 = (glm::dot(glm::vec2(P.x - A.x, P.y - A.y), glm::vec2(A.y - B.y, B.x - A.x)) > 0);
		bool b1 = (glm::dot(glm::vec2(P.x - B.x, P.y - B.y), glm::vec2(B.y - C.y, C.x - B.x)) > 0);
		bool b2 = (glm::dot(glm::vec2(P.x - C.x, P.y - C.y), glm::vec2(C.y - A.y, A.x - C.x)) > 0);
		if (b0 == b1 && b1 == b2) {
			return distAlongRay;
		}
	}
	return -1.0f;
}

float intersectSphere(const Camera::Ray& ray, const Shape& object) {
	float a = glm::dot(ray.dir, ray.dir);
	float b = 2 * glm::dot(ray.dir, (ray.origin - object.center));
	float c = glm::dot((ray.origin - object.center), (ray.origin - object.center)) - glm::pow(object.radius, 2);
	float discrim = calculateDiscriminant(a, b, c);

	if (discrim < 0) {
		return -1.0f;
	}
	float x1 = (-b + glm::sqrt(discrim)) / 2 * a;
	float x2 = (-b - glm::sqrt(discrim)) / 2 * a;
	if (x1 >= 0.0f && x2 >= 0.0f) {
		//Get smallest positive number
		return std::min(x1, x2);
	}
	//One is negative so return the maximum number which should be positive
	return std::max(x1, x2);
}

float intersect(const Camera::Ray& ray, const Shape& object) {
	if (object.isTriangle) {
		return intersectTriangle(ray, object);
	}
	return intersectSphere(ray, object);
}

int findClosestObjectIndex(const Camera::Ray& ray, const std::vector<Shape*>& objects) {
	int closestObjectIndex = -1;
	//Could use a really big number but it's theoretically possible that the intersect 
	//could be very large so using a negative number which it should never be
	float closestHit = -1.0f;
	for (int i = 0; i < objects.size(); i++) {
		float hit = intersect(ray, *objects[i]);
		if (hit >= 0.0f && (hit < closestHit || closestHit < 0.0f)) {
			closestHit = hit;
			closestObjectIndex = i;
		}
	}
	return closestObjectIndex;
}

std::vector<Shape*>& getTestSceneObjects(int scene) {
	std::vector<Shape*>* outObjects = new std::vector<Shape*>();
	switch (scene) {
		case 1: {
			glm::vec3 v0(-1, -1, 0);
			glm::vec3 v1(1, -1, 0);
			glm::vec3 v2(1, 1, 0);
			glm::vec3 v3(-1, 1, 0);
			Shape* tri1 = new Shape(v0, v1, v2, Color(0xFF, 0x00, 0x00));
			Shape* tri2 = new Shape(v0, v2, v3, Color(0xFF, 0x00, 0x00));
			outObjects->push_back(tri1);
			outObjects->push_back(tri2);
			break;
		}
		default: {
			Shape* sphere1 = new Shape(glm::vec3(0, 0, -10), 1.0f, Color(0x5c, 0x42, 0xf4));
			Shape* sphere2 = new Shape(glm::vec3(1, 0, -7), 1.0f, Color(0xff, 0x42, 0xf4));
			Shape* tri1 = new Shape(glm::vec3(-1, 0, -4), glm::vec3(0, 1, -4), glm::vec3(1, 0, -4), Color(0xFF, 0xFF, 0xFF));
			Shape* tri2 = new Shape(glm::vec3(0, 2, -3), glm::vec3(0, 3, -3), glm::vec3(1, 2, -3), Color(0x00, 0xFF, 0xFF));
			outObjects->push_back(sphere1);
			outObjects->push_back(sphere2);
			outObjects->push_back(tri1);
			outObjects->push_back(tri2);
			break;
		}
	}
	return *outObjects;
}

Camera& getTestSceneCamera(int scene, int camera) {
	Camera* cam;
	switch (scene) {
		case 1:
			switch (camera) {
				//Angle seems a little strange for cam 3 and cam 1.
				case 0:
					cam = new Camera(glm::vec3(0, 0, 4), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 30);
					break;
				case 1:
					cam = new Camera(glm::vec3(0, -3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 30.0f);
					break;
				case 2:
					cam = new Camera(glm::vec3(-4, 0, 1), glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), 45);
					break;
				case 3:
					cam = new Camera(glm::vec3(-4, -4, 4), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), 30);
					break;
				default:
					//My test camera
					cam = new Camera(glm::vec3(0, 0, 0), glm::vec3(0, 0, -2), glm::vec3(0, 1, 0), 30);
					break;
			}
			break;
		default:
			//My test camera again
			cam = new Camera(glm::vec3(0, 0, 0), glm::vec3(0, 0, -2), glm::vec3(0, 1, 0), 30);
			break;
	}
	return *cam;
}

//TODO: Fix pixel coloring. Implement depth with multiple spheres. Implement materials. Implement triangle ray intersection. Transformations.

int main(int argc, char* argv[]) {
	/*
	 DONE Fix pixel coloring. 
	 DONE Implement depth with multiple spheres. 
	 Implement materials. 
	 DONE Implement triangle ray intersection. 
	 Transformations.
	 Figure out why camera 2 and 4 looks off in scene1.

		for each pixel
		DONE	compute ray
			find interesctions
			determine color
		render

		TODO:
		DONE	Compute rays
		DONE	Build object intersection for sphere
		DONE	Build object interesction for triangle
		DONE	Adapt for multiple objects and depth testing
			Transformations
			Implement Lighting
			Implement file reading
	*/
	const unsigned int w = 640;
	const unsigned int h = 480;
	BYTE pixels[w*h * 3];
	Color backgroundColor = Color(0, 0, 0);
	Color pixelColor;

	std::vector<Shape*> objects = getTestSceneObjects(1);
	Camera cam = getTestSceneCamera(1, 1);
	
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			Camera::Ray ray = cam.createRay(j + 0.5, i + 0.5, w, h);
			int closestIndex = findClosestObjectIndex(ray, objects);
			if (closestIndex < 0) {
				pixelColor = backgroundColor;
			}
			else {
				pixelColor = objects[closestIndex]->color;
			}
			
			pixels[i*w * 3 + j * 3] = pixelColor.b;
			pixels[i*w* 3 + (j * 3) + 1] = pixelColor.g;
			pixels[i*w* 3 + (j * 3) + 2] = pixelColor.r;
		}
	}

	Renderer render(w, h);
	render.createImage(pixels, "test.png");
	std::cin.get();
	return 0;
}



