#pragma once
#ifndef RENDERER_CLASS_H
#define RENDERER_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Objects.h"
#include "Shader.h"
#include "Scene.h"

class Renderer
{
	Shader m_raytraceShader;
	Shader m_averageShader;
	Shader m_drawTextureShader;

	GLuint renderFBO;
	Texture renderTexture;
	GLuint finalRenderFBO;
	Texture finalRenderTexture;

	unsigned int frame = 0;

public:
	// Raytracing settings
	int maxBounces = 12;
	int samplesPerPixel = 1;
	float perspectiveSlope = 1.2f;
	float focalDistance = 1.0f;
	float focalBlur = 0.0f;
	float blur = 0.0f;
	bool renderMode = false;

	void Initialize(int width, int height);
	void Uninitialize();

	// Set the gl viewport resolution, and updates all the screen textures
	void SetViewportResolution(int width, int height);
	// Get the render shader id
	int GetRenderShaderID();

	// Uploads the raytrace settings to the GPU
	void UploadRaytraceSettings();
	// Upload all the scene spheres, triangles and boundingBoxed to the GPU
	void UploadObjects(Scene& scene);
	// Upload the camera matrix and position to the GPU
	void UploadCameraView(Scene& scene);

	// Get the frame of the current render
	std::vector<float> GetCurrentFrame(int& width, int& height);

	// Render the next frame
	void Render(Scene& scene, bool& sceneChanged);
};

#endif