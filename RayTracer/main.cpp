#include <iostream>
#include <algorithm>
#include <vector>
#include <stack>

#include "FreeImage.h"
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

#include "Renderer.h"
#include "Camera.h"
#include "SceneObjects.hpp"
#include "Scene.h"
bool debugLight = false;
bool debugShadows = false;
bool debugIntersect = false;
bool debugNormals = false;
std::string testFile = "test_scenes/scene3_light.test";
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

std::ostream& operator<<(std::ostream &strm, const glm::vec3 &v1) {
	return strm << "Vector(x: " << v1.x << " y: " << v1.y << " z: " << v1.z << ")";
}

std::ostream& operator<<(std::ostream &strm, const Camera &cam) {
	return strm << "Camera(lookAt: " << cam.lookAt << " lookFrom: " << cam.lookFrom << " up: " << cam.up << " forward: " << cam.forward << " fov: " << cam.fovy << ")";
}

Color operator*(const Color& color, float x) {
	return Color(color.r * x, color.g * x, color.b * x);
}

Color operator*(float x, const Color& color) {
	return color * x;
}

Color operator*(const Color& color1, const Color& color2) {
	return Color(color1.r * color2.r, color1.g * color2.g, color1.b * color2.b);
}

/*Color& operator+=(Color& color1, const Color& color2) {
	color1.r += color2.r;
	color1.g += color2.g;
	color1.b += color2.b;
	return color1;
}*/

float calculateDiscriminant(float a, float b, float c) {
	return glm::pow(b, 2) - (4 * a*c);
}

Intersection intersectTriangle(const Camera::Ray& ray, const Shape& tri) {
	glm::vec3 e1 = tri.v3 - tri.v2;
	glm::vec3 e2 = tri.v1 - tri.v3;
	glm::vec3 e3 = tri.v2 - tri.v1;
	glm::vec3 planeNormal = glm::cross(e1, e2);
	if (!glm::epsilonEqual(glm::dot(ray.dir, planeNormal), 0.0f, 0.01f)) {
		float d = glm::dot(tri.v1, planeNormal);
		float t = (d - glm::dot(ray.origin, planeNormal)) / glm::dot(ray.dir, planeNormal);
		if (t < 0.005f) {
			return Intersection();
		}
		glm::vec3 P = ray.origin + t*ray.dir;
		glm::vec3 d1 = P - tri.v1;
		glm::vec3 d2 = P - tri.v2;
		glm::vec3 d3 = P - tri.v3;
		float totalArea = glm::dot(glm::cross(e1, e2), planeNormal) / 2;
		bool b0 = (glm::dot(glm::cross(e1, d3), planeNormal)/2 / totalArea) >= 0;
		bool b1 = (glm::dot(glm::cross(e2, d1), planeNormal)/2 / totalArea) >= 0;
		bool b2 = (glm::dot(glm::cross(e3, d2), planeNormal)/2 / totalArea) >= 0;
		if (b0 && b1 && b2) {
			return Intersection(glm::distance(P, ray.origin), planeNormal);
		}
	}
	return Intersection();
}

Intersection intersectSphere(const Camera::Ray& rawRay, const Shape& object) {
	Camera::Ray ray(glm::inverse(object.transform) * glm::vec4(rawRay.origin, 1.0f), glm::normalize(glm::inverse(object.transform) * glm::vec4(rawRay.dir, 0.0f)));
	float a = glm::dot(ray.dir, ray.dir);
	float b = 2.0f * glm::dot(ray.dir, (ray.origin - object.center));
	float c = glm::dot((ray.origin - object.center), (ray.origin - object.center)) - glm::pow(object.radius, 2.0f);
	float discrim = calculateDiscriminant(a, b, c);
	if (discrim < 0.0f) {
		return Intersection();
	}
	float x1 = (-b + glm::sqrt(discrim)) / 2.0f * a;
	float x2 = (-b - glm::sqrt(discrim)) / 2.0f * a;
	float t = std::min(x1, x2);
	if (t < 0.005f) {
		//One is negative so return the maximum number which should be positive
		return Intersection();
	}
	glm::vec3 transfPoint = ray.origin + ray.dir * t;
	glm::vec3 normal = glm::transpose(glm::inverse(object.transform)) * glm::vec4(transfPoint - object.center, 0.0f);

	glm::vec3 finalPoint = object.transform * glm::vec4(transfPoint, 1.0f);
	return Intersection(glm::distance(finalPoint, rawRay.origin), normal);
}

Intersection intersect(const Camera::Ray& ray, const Shape& object) {
	if (object.isTriangle) {
		return intersectTriangle(ray, object);
	}
	return intersectSphere(ray, object);
}

Intersection findClosestIntersection(const Camera::Ray& ray, const std::vector<Shape*>& objects) {
	Intersection objIntersect;

	for (int i = 0; i < objects.size(); i++) {
		Intersection currentIntersect = intersect(ray, *objects[i]);
		if (currentIntersect.isValidIntersection()&& (currentIntersect.distAlongRay < objIntersect.distAlongRay || !objIntersect.isValidIntersection())) {
			objIntersect = currentIntersect;
			objIntersect.objectIndex = i;
		}
	}
	return objIntersect;
}

std::vector<Shape*>& getTestSceneObjects(int scene) {
	std::vector<Shape*>* outObjects = new std::vector<Shape*>();
	switch (scene) {
		case 1: {
			glm::vec3 v0(-1, -1, 0);
			glm::vec3 v1(1, -1, 0);
			glm::vec3 v2(1, 1, 0);
			glm::vec3 v3(-1, 1, 0);
			Shape* tri1 = new Shape(v0, v1, v2, Material(Color(0xFF, 0x00, 0x00)));
			Shape* tri2 = new Shape(v0, v2, v3, Material(Color(0xFF, 0x00, 0x00)));
			outObjects->push_back(tri1);
			outObjects->push_back(tri2);
			break;
		}
		default: {
			Shape* sphere1 = new Shape(glm::vec3(0, 0, -10), 1.0f, Material(Color(0x5c, 0x42, 0xf4)));
			Shape* sphere2 = new Shape(glm::vec3(1, 0, -7), 1.0f, Material(Color(0xff, 0x42, 0xf4)));
			Shape* tri1 = new Shape(glm::vec3(-1, 0, -4), glm::vec3(0, 1, -4), glm::vec3(1, 0, -4), Material(Color(0xFF, 0xFF, 0xFF)));
			Shape* tri2 = new Shape(glm::vec3(0, 2, -3), glm::vec3(0, 3, -3), glm::vec3(1, 2, -3), Material(Color(0x00, 0xFF, 0xFF)));
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

//TODO: Fix these color multiplications and probably +=
//TODO: Adjust color calculation
float calculateDiffuseLighting(const glm::vec3& normal, const glm::vec3& objToLightDir) {
	return std::max(glm::dot(glm::normalize(normal), glm::normalize(objToLightDir)), 0.0f);
}

float calculateSpecularLighting(const Material& objMat, const glm::vec3& normal, const glm::vec3& halfAngle) {
	return glm::pow(std::max(glm::dot(halfAngle, normal), 0.0f), objMat.shininess);
}

float calculateLightIntensity(const glm::vec3& attenuation, float distance) {
	return attenuation.x + attenuation.y*distance + attenuation.z*glm::pow(distance, 2.0f);
}

Color calculatePixelColor(const Scene& scene, const glm::vec3& intersectPoint, const glm::vec3& intersectNormal, const Material& objMat) {
	Color colorFromLights = objMat.ambient + objMat.emission;
	Color diffuseLightColor;
	Color specularLightColor;
	int j = 0;
	for (Light light : scene.getLights()) {
		glm::vec3 lightRayDir;
		glm::vec3 lightDir;
		float distance;
		if (light.isPointLight()) {
			lightRayDir = glm::vec3(light.location) - intersectPoint;
			lightDir = lightRayDir;
			distance = glm::length(lightRayDir);
		}
		else {
			lightRayDir = -glm::vec3(light.location);
			lightDir = -glm::vec3(light.location);
			distance = 0.0f;
		}
		Camera::Ray ray(intersectPoint, lightRayDir);
		Intersection intersect = findClosestIntersection(ray, scene.getSceneObjects());
		//Differentiate directional and point lights when calculating ray direction
		if (!intersect.isValidIntersection()) {

			float atten = calculateLightIntensity(scene.attenuation, distance);
			float lightPercent = calculateDiffuseLighting(intersectNormal, lightDir);
			if (debugLight) {
				colorFromLights += Color(lightPercent, lightPercent, lightPercent);
			}
			else if (debugNormals) {
				colorFromLights += Color(intersectNormal.x, intersectNormal.y, intersectNormal.z);
			}
			else {
				diffuseLightColor += atten * lightPercent * light.color;

				glm::vec3 eyeDir = glm::normalize(scene.getCamera().getEye() - intersectPoint);
				glm::vec3 halfAngle = glm::normalize(lightDir + eyeDir);
				specularLightColor += atten * calculateSpecularLighting(objMat, intersectNormal, halfAngle) * light.color;
			}
		}
		else {
			if (debugShadows) {
				colorFromLights += scene.getSceneObjects()[intersect.objectIndex]->material.diffuse;
			}
		/*	if (intersect.objectIndex == 1) {
				colorFromLights += Color(1.0f, 0.0f, 1.0f);
			}
			else {
				colorFromLights += Color(1.0f, 1.0f, 0.0f);
			} */
			//colorFromLights += Color(ray.dir.x, ray.dir.y, ray.dir.z);
		}
	}
	colorFromLights += objMat.diffuse * diffuseLightColor;
	colorFromLights += objMat.specular * specularLightColor;
	return colorFromLights;
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
		DONE	Transformations
			Normals and transformed normals (inverse transpose)
			Implement Lighting
			Implement file reading
			Implement shadows
			Implement reflection
	*/
	Scene scene(testFile);
	unsigned int w = scene.getWidth();
	unsigned int h = scene.getHeight();

	std::vector<BYTE> pixels(w*h * 3);
	Color backgroundColor = Color(0, 0, 0);
	Color pixelColor;

	std::vector<Shape*> objects = scene.getSceneObjects();
	Camera cam = scene.getCamera();
	std::cout << cam << std::endl;
	
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			Camera::Ray ray = cam.createRayToPixel(j + 0.5, i + 0.5, w, h);
			Intersection closestIntersect = findClosestIntersection(ray, objects);
			if (!closestIntersect.isValidIntersection()) {
				pixelColor = backgroundColor;
			}
			else {
				//calculate lighting by casting ray to all lights and taking material colors into consideration, if the ray is obscured the pixel is "in shadow" meaning it doesn't take color from that light,
				//if no lights light the object, it'll be the ambient color
				//Then cast reflection ray to intersect with another object, pixel being rendered takes on color from that object
				if (debugIntersect) {
					pixelColor = Color(1.0f, 0.0f, 0.0f);
				}
				else {
					pixelColor = calculatePixelColor(scene, Camera::createPointFromRay(ray, closestIntersect.distAlongRay), closestIntersect.intersectNormal, objects[closestIntersect.objectIndex]->material);
				}
			}
			pixels[i*w * 3 + j * 3] = pixelColor.getB();
			pixels[i*w* 3 + (j * 3) + 1] = pixelColor.getG();
			pixels[i*w* 3 + (j * 3) + 2] = pixelColor.getR();
		}
	}

	Renderer render(w, h);
	BYTE * outPixels = &pixels[0];
	render.createImage(outPixels, scene.getOutputFileName());
	std::cin.get();
	return 0;
}



