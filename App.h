#pragma once
#ifndef APP_CLASS_H
#define APP_CLASS_H

#define _CRT_SECURE_NO_WARNINGS

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Windows.h>
#include <chrono>
#include "GUI.h"
#include "Renderer.h"
#include "Scene.h"

class App
{
private:
	GLFWwindow* m_window;
	unsigned int m_windowWidth, m_windowHeight;
	std::string m_windowTitle;

	double m_prevTime = 0.0, m_crntTime, m_deltaTime = 0.0, m_fTheta = 0.0;
	unsigned int m_frameCount = 0;
	
	bool m_viewChanged = false;

public:
	Renderer renderer;
	Scene scene;

	App(int width, int height, const std::string& title);
	~App();

	// For handling window resizing
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void HandleWindowResize(int newWidth, int newHeight);

	// Update the fps, and deltaTime
	void UpdateTimer();
	// The main game loop
	int Start();

	// Load the spheres, triangles and calculate the bounding boxes
	void LoadScene();
	// Save the current render
	void ScreenShot();
};

#endif