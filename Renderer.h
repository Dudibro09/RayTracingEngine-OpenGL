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
	void Initialize(int width, int height);
	void Uninitialize();

	void SetViewportResolution(int width, int height);

	// Upload the scene spheres, triangles and boundingBoxed to the GPU
	void UploadScene(Scene& scene);
	// Uploads the raytrace settings to the GPU
	void UploadRaytraceSettings(int maxBounces, int samplesPerPixel, float perspectiveSlope, float focalDistance, float focalBlur, float blur);
	// Upload the camera matrix and position to the GPU
	void UploadCameraView(Scene& scene);
	// Get the frame of the current render
	std::vector<float> GetCurrentFrame(int& width, int& height);

	// Render to the window
	void Render(const Scene& scene, bool& sceneChanged);
};

#endif