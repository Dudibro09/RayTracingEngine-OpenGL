#pragma once
#ifndef OBJECTS_CLASS_H
#define OBJECTS_CLASS_H

#include <glm/glm.hpp>
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

private:
	float padding = 0.0f;
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

	Material material;
};

struct BoundingBox
{
	glm::vec3 min = glm::vec3(std::numeric_limits<float>::infinity());
	float padding = 0.0f;

	glm::vec3 max = glm::vec3(-std::numeric_limits<float>::infinity());
	float padding2 = 0.0f;

	int triangleIndex = -1;
	int boundingBoxAIndex = -1;
	int boundingBoxBIndex = -1;
	float padding3 = 0.0f;

	void GrowToInclude(const glm::vec3& point);
	void GrowToInclude(const Triangle& triangle);
};

#endif