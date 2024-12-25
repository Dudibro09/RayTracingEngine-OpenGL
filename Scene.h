#pragma once
#ifndef SCENE_CLASS_H
#define SCENE_CLASS_H

#include "Objects.h"
#include "Camera.h"
#include "Texture.h"
#include "Shader.h"

class Scene
{
private:
	GLuint trianglesSSBO;
	GLuint spheresSSBO;
	GLuint boundingBoxesSSBO;
	GLuint sortedTriangleIndicesSSBO;

	void RecursiveBoundingBox(int boundingBoxIndex, std::vector<int>& triangleIndices, int depth);

public:
	std::vector<Sphere> spheres;
	std::vector<Triangle> triangles;
	std::vector<BoundingBox> boundingBoxes;
	std::vector<int> sortedTriangleIndices;
	Texture skybox;
	Camera camera;

	void Initialize();
	void Uninitialize();

	// Loads a mesh to the triangle array
	bool LoadMesh(const char* path, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, const Material& material);
	// Precalculates the bounding boxes of all the triangles to drastically improve performance, but makes it hard to dynamically modify the scene triangles
	void UpdateBoundingBox();
	// Updates the shader storage buffer with the scene data
	void UpdateSSBO(GLuint shaderID);
};

#endif