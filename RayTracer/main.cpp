#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <stack>
#include <stdlib.h>
#include <time.h>
#include <unordered_map>
#include <windows.h>
#include "Shlwapi.h"

#include "FreeImage.h"
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

#include "Renderer.h"
#include "Camera.h"
#include "SceneObjects.hpp"
#include "Scene.h"

#pragma comment(lib, "Shlwapi.lib")


SceneMetaData createSceneMetaData(const std::string& sceneFilePath) {
	SceneMetaData metaData(sceneFilePath, PathFindFileName(sceneFilePath.c_str()));
	return metaData;
}

enum class Debug {
	//Debug flags aren't assigned numbers to make them easier to iterate through
	DIFFUSE_LIGHT_INTENSITY,
	SPECULAR_LIGHT_INTENSITY,
	NORMAL_MAP,
	SHADOW_MAP,
	PRIMARY_INTERSECTION_MAP,
	NONE //NONE should be last to make iterating through these debugs easier since loops will stop on NONE
};

enum class Feature {
	DIFFUSE_LIGHTING = 1,
	SPECULAR_LIGHTING = 2,
	SHADOWS = 4,
	REFLECTIONS = 8,
	KEEP_TIME = 16,
	REPORT_PERFORMANCE = 32
};

enum class Mode {
	BENCHMARK,
	NONE
};

std::unordered_map<Debug, std::string> debugNames({ { Debug::DIFFUSE_LIGHT_INTENSITY, "diffuse_intensity" },{ Debug::SPECULAR_LIGHT_INTENSITY, "specular_intensity" },{ Debug::NORMAL_MAP, "normals" },{ Debug::PRIMARY_INTERSECTION_MAP, "primary_intersect" },{ Debug::SHADOW_MAP, "shadow_intersect" },{ Debug::NONE, "none" } });
std::unordered_map<Feature, std::string> featureNames({ { Feature::DIFFUSE_LIGHTING, "diffuse" }, {Feature::SPECULAR_LIGHTING, "specular"}, {Feature::REFLECTIONS, "reflections"}, {Feature::SHADOWS, "shadows"},{ Feature::KEEP_TIME, "time" },{ Feature::REPORT_PERFORMANCE, "reporting" } });
int featureFlags = (int)Feature::DIFFUSE_LIGHTING | (int)Feature::SPECULAR_LIGHTING | (int)Feature::SHADOWS | (int)Feature::KEEP_TIME | (int)Feature::REPORT_PERFORMANCE;
Debug debugFlag = Debug::NONE;
Mode currentMode = Mode::NONE;

void removeFeature(Feature feature) {
	featureFlags ^= (int)feature;
}

void addFeature(Feature feature) {
	featureFlags |= (int)feature;
}

bool featureIsActive(Feature requestedFeature) {
	return featureFlags & (int)requestedFeature;
}

bool debugIsActive(Debug requestedDebug) {
	return debugFlag == requestedDebug;
}

std::string getEnabledFeaturesAsString() {
	std::string featureStr;
	for (auto &feature : featureNames) {
		if (featureIsActive(feature.first)) {
			if (featureStr.empty()) {
				featureStr += feature.second;
			}
			else {
				featureStr += " " + feature.second;
			}
		}
	}
	return featureStr;
}

std::string getEnabledDebugAsString() {
	return debugNames.find(debugFlag)->second;
}

double sampleTimeInSeconds = 5.0;
std::string testScenesDirectory = "test_scenes/";
std::string reportDirectory = "reports/";
std::string renderDirectory = "renders/";
std::string debugRenderDirectory = "debug_renders/";
std::string testFile = "scene1.test";
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

float calculateDiffuseLighting(const glm::vec3& normal, const glm::vec3& objToLightDir) {
	return std::max(glm::dot(glm::normalize(normal), glm::normalize(objToLightDir)), 0.0f);
}

float calculateSpecularLighting(const Material& objMat, const glm::vec3& normal, const glm::vec3& halfAngle) {
	return glm::pow(std::max(glm::dot(halfAngle, normal), 0.0f), objMat.shininess);
}

float calculateLightIntensity(const glm::vec3& attenuation, float distance) {
	return attenuation.x + attenuation.y*distance + attenuation.z*glm::pow(distance, 2.0f);
}

Color calculateLightingColor(const Scene& scene, const glm::vec3& intersectPoint, const glm::vec3& intersectNormal, const Material& objMat) {
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
		if (!intersect.isValidIntersection() || !featureIsActive(Feature::SHADOWS)) {

			float atten = calculateLightIntensity(scene.attenuation, distance);
			float diffuseLightIntensity = calculateDiffuseLighting(intersectNormal, lightDir);
			glm::vec3 eyeDir = glm::normalize(scene.getCamera().getEye() - intersectPoint);
			glm::vec3 halfAngle = glm::normalize(lightDir + eyeDir);
			float specularLightIntensity = calculateSpecularLighting(objMat, intersectNormal, halfAngle);
			if (debugIsActive(Debug::DIFFUSE_LIGHT_INTENSITY)) {
				colorFromLights += Color(diffuseLightIntensity, diffuseLightIntensity, diffuseLightIntensity);
			}
			else if (debugIsActive(Debug::SPECULAR_LIGHT_INTENSITY)) {
				colorFromLights += Color(specularLightIntensity, specularLightIntensity, specularLightIntensity);
			}
			else if (debugIsActive(Debug::NORMAL_MAP)) {
				colorFromLights += Color(intersectNormal.x, intersectNormal.y, intersectNormal.z);
			}
			else {
				if (featureIsActive(Feature::DIFFUSE_LIGHTING)) {
					diffuseLightColor += atten * diffuseLightIntensity * light.color;
				}
				if (featureIsActive(Feature::SPECULAR_LIGHTING)) {
					specularLightColor += atten * specularLightIntensity * light.color;
				}
			}
		}
		else if (debugIsActive(Debug::SHADOW_MAP)) {
				colorFromLights += scene.getSceneObjects()[intersect.objectIndex]->material.diffuse;
		}
	}
	colorFromLights += objMat.diffuse * diffuseLightColor;
	colorFromLights += objMat.specular * specularLightColor;
	return colorFromLights;
}

Color computePixelColor(const Camera::Ray& ray, const Scene& scene, int currentDepth) {
	std::vector<Shape*> objects = scene.getSceneObjects();
	if (currentDepth <= scene.maxDepth) {
		Intersection closestIntersect = findClosestIntersection(ray, objects);
		if (!closestIntersect.isValidIntersection()) {
			return scene.backgroundColor;
		}
		else {
			//calculate lighting by casting ray to all lights and taking material colors into consideration, if the ray is obscured the pixel is "in shadow" meaning it doesn't take color from that light,
			//if no lights light the object, it'll be the ambient color
			//Then cast reflection ray to intersect with another object, pixel being rendered takes on color from that object
			if (debugIsActive(Debug::PRIMARY_INTERSECTION_MAP)) {
				return Color(1.0f, 0.0f, 0.0f);
			}
			else {
				Color lightColor = calculateLightingColor(scene, Camera::createPointFromRay(ray, closestIntersect.distAlongRay), closestIntersect.intersectNormal, objects[closestIntersect.objectIndex]->material);
				Camera::Ray reflectRay(Camera::createPointFromRay(ray, closestIntersect.distAlongRay), ray.dir - 2.0f*glm::dot(ray.dir, closestIntersect.intersectNormal)*closestIntersect.intersectNormal);
				if (featureIsActive(Feature::REFLECTIONS)) {
					return lightColor + 0.8*objects[closestIntersect.objectIndex]->material.specular*computePixelColor(reflectRay, scene, ++currentDepth);
				}
				else {
					return lightColor;
				}
			}
		}
	}
	else {
		return Color();
	}
}

void createPerformanceReport(const SceneMetaData& metaData, const std::string& outputFileName, const Scene& scene, const time_t& totalTimeInSeconds, int pixelsProcessed) {
	std::ofstream report;
	SceneMetaData outputMeta=createSceneMetaData(outputFileName);
	report.open(reportDirectory + outputMeta.sceneTitle + "_report.txt");
	if (currentMode == Mode::BENCHMARK) {
		report << "BENCHMARK RUN: MAY HAVE EXITED BEFORE COMPLETELY RENDERING" << std::endl;
	}
	report << "PERFORMANCE REPORT FOR " << outputMeta.sceneTitle << std::endl;
	report << "--------------------------------------------------------------------" << std::endl << std::endl;
	report << std::min((int)((pixelsProcessed / (float)scene.getWidth() * scene.getHeight()) * 100), 100) << "% Completed" << std::endl << std::endl;
	report << "Input Scene File: " << metaData.filePath << std::endl;
	report << "Output Image: " << outputFileName << std::endl;
	report << "Resolution: " << scene.getWidth() << "x" << scene.getHeight() << std::endl;
	report << "Pixels Processed: " << pixelsProcessed << std::endl << std::endl;
	report << "Features Enabled: " << getEnabledFeaturesAsString() << std::endl;
	report << "Debug Options: " << getEnabledDebugAsString() << std::endl << std::endl;
	char buffer[80];
	struct tm timeInfo = { 0 };
	localtime_s(&timeInfo, &totalTimeInSeconds);
	strftime(buffer, 80, "%H hours %M minutes %S seconds", &timeInfo);
	report << "Render Time: " << buffer << std::endl;
	report << "Milliseconds Per Pixel: " << totalTimeInSeconds * 1000 / (float)pixelsProcessed << std::endl << std::endl;
	report << "Total objects: " << scene.getNumObjects() << std::endl;
	report << "-----Spheres: " << scene.getNumSpheres() << std::endl;
	report << "-----Triangles: " << scene.getNumTriangles() << std::endl;
	report << "Total lights: " << scene.getNumLights() << std::endl;
	report << "-----Directional: " << scene.getNumDirectionalLights() << std::endl;
	report << "-----Point: " << scene.getNumPointLights() << std::endl;
	report.close();
}

void createRender(const SceneMetaData& sceneFileData, std::string outputFileName="") {
	std::string testFilePath = sceneFileData.filePath;
	srand(NULL);
	Scene scene(testFilePath);
	if (!scene.loaded()) {
		std::cout << "Couldn't load scene. Is the file path correct? " << testFilePath << std::endl;
		std::cin.get();
		exit(1);
	}
	if (outputFileName.empty()) {
		outputFileName = scene.getOutputFileName();
	}
	else {
		outputFileName += ".png";
	}

	unsigned int w = scene.getWidth();
	unsigned int h = scene.getHeight();

	std::vector<BYTE> pixels(w*h * 3);
	scene.backgroundColor = Color(0, 0, 0);
	Color pixelColor;
	float widthOffset = 0.0f;
	float heightOffset = 0.0f;
	std::vector<Shape*> objects = scene.getSceneObjects();
	Camera cam = scene.getCamera();
	unsigned int total = w * h;
	time_t startTime = time(NULL);
	time_t lastSampleTime = startTime;
	double benchmarkTimeLimit = 60.0f*60.0f*30.0f; //30 minutes
	struct tm sample = { 0 };
	sample.tm_sec = sampleTimeInSeconds;
	int currentPixel;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			currentPixel = i * w + j;
			if (featureIsActive(Feature::KEEP_TIME)) {
				double seconds = difftime(time(NULL), lastSampleTime);
				if (seconds > sampleTimeInSeconds) {
					lastSampleTime = time(NULL);
					float percentComplete = (currentPixel / (float)total) * 100.0f;
					double totalTime = difftime(lastSampleTime, startTime);
					float estTime = ((float)total - currentPixel) / (currentPixel / totalTime);
					std::cout << percentComplete << "% complete. Estimated time: " << estTime << " seconds" << std::endl;
				}
			}
			widthOffset = (rand() % 50) / 100.0f;
			heightOffset = (rand() % 50) / 100.0f;
			Camera::Ray ray = cam.createRayToPixel(j + widthOffset, i + heightOffset, w, h);
			pixelColor = computePixelColor(ray, scene, 1);
			pixels[i*w * 3 + j * 3] = pixelColor.getB();
			pixels[i*w * 3 + (j * 3) + 1] = pixelColor.getG();
			pixels[i*w * 3 + (j * 3) + 2] = pixelColor.getR();
		}
		if (currentMode == Mode::BENCHMARK) {
			if (difftime(time(NULL), startTime) > benchmarkTimeLimit) {
				break;
			}
		}
	}
	++currentPixel;
	if (currentPixel == w * h) {
		Renderer render(w, h);
		BYTE * outPixels = &pixels[0];
		render.createImage(outPixels, outputFileName);
	}
	if (featureIsActive(Feature::REPORT_PERFORMANCE)) {
		time_t totalTime = time(NULL) - startTime;
		createPerformanceReport(sceneFileData, outputFileName, scene, totalTime, currentPixel);
	}
}

void createAllDebugRendersForScene(const SceneMetaData& metaData) {
	for (int flag = 1; flag != (int)Debug::NONE; flag++) {
		debugFlag = (Debug)flag;
		createRender(metaData, debugRenderDirectory+"debug_"+debugNames.find(debugFlag)->second+metaData.sceneTitle);
	}
}

void createAllFeatureRendersForScene(const SceneMetaData& metaData) {
	debugFlag = Debug::NONE;
	featureFlags = (int)Feature::KEEP_TIME | (int)Feature::REPORT_PERFORMANCE;
	addFeature(Feature::DIFFUSE_LIGHTING);
	createRender(metaData, renderDirectory+"diffuse_only_" + metaData.sceneTitle);
	addFeature(Feature::SPECULAR_LIGHTING);
	createRender(metaData, renderDirectory+"no_shadows_" + metaData.sceneTitle);
	addFeature(Feature::SHADOWS);
	createRender(metaData, renderDirectory+"full_phong_" + metaData.sceneTitle);
	removeFeature(Feature::SHADOWS);
	addFeature(Feature::REFLECTIONS);
	createRender(metaData, renderDirectory+"reflections_no_shadows_" + metaData.sceneTitle);
	addFeature(Feature::SHADOWS);
	createRender(metaData, renderDirectory+"all_features_" + metaData.sceneTitle);
}

void createAllRendersForScene(const SceneMetaData& metaData) {
	createAllDebugRendersForScene(metaData);
	createAllFeatureRendersForScene(metaData);
}

void createAllFeatureRendersForScene(const std::string& sceneFile) {
	SceneMetaData metaData = createSceneMetaData(sceneFile);
	createAllFeatureRendersForScene(metaData);
}

void createAllDebugRendersForScene(const std::string& sceneFile) {
	SceneMetaData metaData = createSceneMetaData(sceneFile);
	createAllDebugRendersForScene(metaData);
}

void createAllRendersForScene(const std::string& sceneFile) {
	SceneMetaData metaData = createSceneMetaData(sceneFile);
	createAllRendersForScene(metaData);
}



int main(int argc, char* argv[]) {
	SceneMetaData metaData = createSceneMetaData("test_scenes/scene1.test");
	//createRender(metaData);
	createAllRendersForScene(metaData);
	std::cout << "Finished Rendering" << std::endl;
	std::cin.get();
	return 0;
}



