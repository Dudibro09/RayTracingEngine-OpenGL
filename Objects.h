#pragma once
#ifndef OBJECTS_CLASS_H
#define OBJECTS_CLASS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glad/glad.h>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <limits>

struct Material
{
	Material() = default;
	Material(glm::vec3 color, float roughness, float emissionStrength, float emissionScatteringIndex, float absorbsionStrength, float refractiveIndex, float reflectiveIndex) :
		color(color), emissionColor(color), absorbColor(glm::vec3(1.0f - color.r, 1.0f - color.g, 1.0f - color.b)),
		roughness(roughness),
		emissionStrength(emissionStrength),
		absorbsionStrength(absorbsionStrength),
		emissionScatteringIndex(emissionScatteringIndex),
		refractiveIndex(refractiveIndex),
		reflectiveIndex(reflectiveIndex)
	{}

	glm::vec3 color = glm::vec3(0.0f);
	float roughness = 0.0f;

	glm::vec3 emissionColor = glm::vec3(0.0f);
	float emissionStrength = 0.0f;

	glm::vec3 absorbColor = glm::vec3(0.0f);
	float absorbsionStrength = 0.0f;

	float emissionScatteringIndex = 0.0f;
	float refractiveIndex = 0.0f;
	float reflectiveIndex = 0.0f;
	float padding;
};

struct Sphere
{
	glm::vec3 position = glm::vec3(0.0f);
	float radius = 0.0f;

	Material material;
};

struct Triangle
{
	glm::vec4 p[3];
};

struct BoundingBox
{
	glm::vec3 min = glm::vec3(std::numeric_limits<float>::infinity());
	float padding;

	glm::vec3 max = glm::vec3(-std::numeric_limits<float>::infinity());
	float padding2;

	int triangleIndex = -1;
	int boundingBoxAIndex = -1;
	int boundingBoxBIndex = -1;
	float padding3;

	void GrowToInclude(const glm::vec3& point);
	void GrowToInclude(const Triangle& triangle);
};

class Mesh
{
	void RecursiveBoundingBox(int boundingBoxIndex, std::vector<int>& triangleIndices);

public:
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	glm::mat4 localToWorldMatrix = glm::mat4(1.0f);
	glm::mat4 modelWorldToLocalMatrix = glm::mat4(1.0f);

	std::vector<Triangle> triangles;
	std::vector<BoundingBox> boundingBoxes;
	Material material;

	// Calculates the transform matrix based on the position, rotation and scale;
	void UpdateTransformMatrix();
	// Loads the mesh from a .obj file, and removes any prexisting triangles
	bool LoadFromObjectFile(const char* file);
	// Pre-computes the bounding boxes of the triangles to speed up rendering. BY A LOT
	void UpdateBoundingBoxes();
};

struct ShaderReadyMesh
{
	glm::mat4 localToWorldMatrix = glm::mat4(0.0f);
	glm::mat4 modelWorldToLocalMatrix = glm::mat4(0.0f);

	int triangleIndex;
	int nTriangles;

	int boundingBoxIndex;
	int nBoundingBoxes;

	Material material;
};

#endif