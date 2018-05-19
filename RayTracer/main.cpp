#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <unordered_map>
#include <windows.h>
#include "Shlwapi.h"

#include "FreeImage.h"
#include <glm/glm.hpp>

#include "Renderer.h"
#include "Camera.h"
#include "SceneObjects.hpp"
#include "Scene.h"
#include "Shape.h"

#pragma comment(lib, "Shlwapi.lib")

SceneMetaData createSceneMetaData(const std::string& sceneFilePath);
inline bool modeIs(Mode mode);
inline void removeFeature(Feature feature);
inline void addFeature(Feature feature);
inline bool featureIsActive(Feature requestedFeature);
inline bool debugIsActive(Debug requestedDebug);
std::string getEnabledFeaturesAsString();
std::string getEnabledDebugAsString();
float calculateDiffuseLighting(const glm::vec3& normal, const glm::vec3& objToLightDir);
float calculateSpecularLighting(const Material& objMat, const glm::vec3& normal, const glm::vec3& halfAngle);
float calculateAttenuation(const glm::vec3& attenuation, float distance);
Color calculateLightingColor(const Scene& scene, const glm::vec3& intersectPoint, const glm::vec3& intersectNormal, const Material& objMat, const glm::vec3& viewPoint);
Color computePixelColor(const Ray& ray, const Scene& scene, int currentDepth, Material baseObjMat = Material());
void createPerformanceReport(const SceneMetaData& metaData, const std::string& outputFileName, const Scene& scene, const time_t& totalTimeInSeconds, int pixelsProcessed);
void createRender(const SceneMetaData& sceneFileData, std::string outputFileName = "");
void createAllDebugRendersForScene(const SceneMetaData& metaData);
void createAllFeatureRendersForScene(const SceneMetaData& metaData);
void createAllRendersForScene(const SceneMetaData& metaData);
void createAllFeatureRendersForScene(const std::string& sceneFile);
void createAllDebugRendersForScene(const std::string& sceneFile);
void createAllRendersForScene(const std::string& sceneFile);

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
	LIGHT_DIRECTION_MAP,
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
//
std::unordered_map<Debug, std::string> debugNames({ { Debug::DIFFUSE_LIGHT_INTENSITY, "diffuse_intensity" },{ Debug::SPECULAR_LIGHT_INTENSITY, "specular_intensity" },{ Debug::NORMAL_MAP, "normals" },{ Debug::PRIMARY_INTERSECTION_MAP, "primary_intersect" },{ Debug::SHADOW_MAP, "shadow_intersect" },{Debug::LIGHT_DIRECTION_MAP, "light_direction_map"}, { Debug::NONE, "none" } });
std::unordered_map<Feature, std::string> featureNames({ { Feature::DIFFUSE_LIGHTING, "diffuse" }, {Feature::SPECULAR_LIGHTING, "specular"}, {Feature::REFLECTIONS, "reflections"}, {Feature::SHADOWS, "shadows"},{ Feature::KEEP_TIME, "time" },{ Feature::REPORT_PERFORMANCE, "reporting" } });
int featureFlags = (int)Feature::DIFFUSE_LIGHTING | (int)Feature::SHADOWS | (int)Feature::SPECULAR_LIGHTING | (int)Feature::KEEP_TIME | (int)Feature::REPORT_PERFORMANCE | (int) Feature::REFLECTIONS;
Debug debugFlag = Debug::NONE;
Mode currentMode = Mode::BENCHMARK;
//spheres, triangles, file reading, shadow calculations, reflection calculations, pixel color calculations
inline bool modeIs(Mode mode) {
	return currentMode == mode;
}

inline void removeFeature(Feature feature) {
	featureFlags ^= (int)feature;
}

inline void addFeature(Feature feature) {
	featureFlags |= (int)feature;
}

inline bool featureIsActive(Feature requestedFeature) {
	return featureFlags & (int)requestedFeature;
}

inline bool debugIsActive(Debug requestedDebug) {
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

Color operator*(float x, const Color& color) {
	return color * x;
}

float calculateDiffuseLighting(const glm::vec3& normal, const glm::vec3& objToLightDir) {
	return std::max(glm::dot(glm::normalize(normal), glm::normalize(objToLightDir)), 0.0f);
}

float calculateSpecularLighting(const Material& objMat, const glm::vec3& normal, const glm::vec3& halfAngle) {
	return glm::pow(std::max(glm::dot(halfAngle, normal), 0.0f), objMat.shininess);
}

float calculateAttenuation(const glm::vec3& attenuation, float distance) {
	return 1.0f/(attenuation.x + attenuation.y*distance + attenuation.z*glm::pow(distance, 2.0f));
}

Color calculateLightingColor(const Scene& scene, const glm::vec3& intersectPoint, const glm::vec3& intersectNormal, const Material& objMat, const glm::vec3& viewPoint) {
	Color colorFromLights = objMat.ambient + objMat.emission;
	Color diffuseLightColor;
	Color specularLightColor;
	int j = 0;
	for (Light light : scene.getLights()) {
		glm::vec3 lightRayDir;
		glm::vec3 lightDir;
		float distance;
		float atten;
		if (light.isPointLight()) {
			lightDir = glm::vec3(light.location) - intersectPoint;
			distance = glm::length(lightDir);
			atten = calculateAttenuation(scene.attenuation, distance);
		}
		else {
			lightDir = glm::vec3(light.location);
			atten = 1.0f;
		}
		Ray ray(intersectPoint, lightDir);
		Intersection intersect = scene.findClosestIntersection(ray);
		//Differentiate directional and point lights when calculating ray direction
		//|| intersect.distAlongRay > glm::length(lightRayDir)
		//|| intersect.distAlongRay >= glm::length(lightDir)
		if (!intersect.isValidIntersection() || intersect.distAlongRay >= glm::length(lightDir) || !featureIsActive(Feature::SHADOWS)) {
			float diffuseLightIntensity = calculateDiffuseLighting(intersectNormal, lightDir);
			//ray.origin
			glm::vec3 eyeDir = viewPoint - intersectPoint;
			//glm::vec3 reflectDir = glm::normalize(lightDir + 2.0f*glm::dot(-lightDir, intersectNormal))*intersectNormal;
			glm::vec3 halfAngle = normalize(lightDir + eyeDir);
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
			else if (debugIsActive(Debug::LIGHT_DIRECTION_MAP)) {
				colorFromLights += Color(halfAngle.x, halfAngle.y, halfAngle.z);
			}
			else {
				if (featureIsActive(Feature::DIFFUSE_LIGHTING)) {
					colorFromLights += atten * objMat.diffuse * diffuseLightIntensity * light.color;
				}
				if (featureIsActive(Feature::SPECULAR_LIGHTING)) {
					colorFromLights += atten * objMat.specular* specularLightIntensity * light.color;
				}
			}
		}
		else if (debugIsActive(Debug::SHADOW_MAP)) {
				colorFromLights += intersect.mat.diffuse;
		}
	}

	return colorFromLights;
}

Color computePixelColor(const Ray& ray, const Scene& scene, int currentDepth, Material baseObjMat = Material()) {
	if (currentDepth <= scene.maxDepth) {
		Intersection closestIntersect = scene.findClosestIntersection(ray);
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
				Color lightColor = calculateLightingColor(scene, Camera::createPointFromRay(ray, closestIntersect.distAlongRay), closestIntersect.intersectNormal, closestIntersect.mat, ray.origin);
				Ray reflectRay(Camera::createPointFromRay(ray, closestIntersect.distAlongRay), ray.dir - 2.0f*glm::dot(ray.dir, closestIntersect.intersectNormal)*closestIntersect.intersectNormal);
				if (featureIsActive(Feature::REFLECTIONS)) {
					if (currentDepth == 0) {
						baseObjMat = closestIntersect.mat;
					}
					return lightColor + baseObjMat.specular*computePixelColor(reflectRay, scene, ++currentDepth, closestIntersect.mat);	
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
	report << "Time Breakdown" << std::endl;
	report << "Total objects: " << scene.getNumObjects() << std::endl;
	report << "----- Spheres: " << scene.getNumSpheres() << std::endl;
	report << "----- Triangles: " << scene.getNumTriangles() << std::endl;
	report << "Total lights: " << scene.getNumLights() << std::endl;
	report << "----- Directional: " << scene.getNumDirectionalLights() << std::endl;
	report << "----- Point: " << scene.getNumPointLights() << std::endl;
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
	std::vector<Shape *> objects = scene.getSceneObjects();
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
			widthOffset = 0.5f;
			heightOffset = 0.5f;
			Ray ray = cam.createRayToPixel(j + widthOffset, i + heightOffset, w, h);
			pixelColor = computePixelColor(ray, scene, 0);
			pixels[i*w * 3 + j * 3] = pixelColor.getB();
			pixels[i*w * 3 + (j * 3) + 1] = pixelColor.getG();
			pixels[i*w * 3 + (j * 3) + 2] = pixelColor.getR();
		}
		if (modeIs(Mode::BENCHMARK)) {
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
	for (int flag = 0; flag != (int)Debug::NONE; flag++) {
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
	SceneMetaData metaData = createSceneMetaData("final_scenes/scene4-specular.test");
	createRender(metaData);
	//createAllRendersForScene(metaData);
	std::cout << "Finished Rendering" << std::endl;
	std::cin.get();
	return 0;
}



