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
	GLuint m_spheresSSBO;
	GLuint m_meshesSSBO;
	GLuint m_trianglesSSBO;
	GLuint m_boundingBoxesSSBO;

	std::vector<ShaderReadyMesh> shaderReadyMeshes;

public:
	std::vector<Sphere> spheres;
	std::vector<Mesh> meshes;

	Texture skybox;
	Camera camera;

	void Initialize();
	void Uninitialize();

	void UpdateMesh(GLuint shaderID, const int& index);
	void UpdateSphere(GLuint shaderID, const int& index);

	void AddMesh(const char* file);

	// Updates the shader storage buffer with the scene data
	void UpdateSSBO(GLuint shaderID);
};

#endif