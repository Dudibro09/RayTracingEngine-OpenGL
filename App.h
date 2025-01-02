#pragma once
#ifndef APP_CLASS_H
#define APP_CLASS_H

#define _CRT_SECURE_NO_WARNINGS

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Windows.h>
#include <chrono>
#include <filesystem>
#include "GUI.h"
#include "Renderer.h"
#include "Scene.h"

class App
{
public:
	App(int width, int height, const std::string& title);
	~App();

	int Start();

private:
	/* GLFW */
	unsigned int m_windowWidth, m_windowHeight;
	std::string m_windowTitle;
	GLFWwindow* m_window;

	double m_prevTime = 0.0, m_crntTime, m_deltaTime = 0.0, m_fTheta = 0.0;
	unsigned int m_frameCount = 0;
	// Update the fps, and deltaTime
	void UpdateTimer();

	// Window callbacks
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void HandleWindowResize(int newWidth, int newHeight);
	static void drop_callback(GLFWwindow* window, int count, const char** paths);
	void HandleFileDragDrop(int count, const char** paths);


	/* RENDERER */
	Renderer m_renderer;
	// Save the current render
	void ScreenShot();


	/* SCENE */
	Scene m_scene;
	// A value to make sure that whenever something in the scene changes, the accumilation starts over
	bool m_sceneChanged = false;
	// Load the spheres, triangles and calculate the bounding boxes
	void LoadScene();


	/* DEAR IMGUI */
	ImGuiIO* m_io;

	// Window states
	bool m_isAddObjectWindowOpen = false;
	std::vector<SceneObject> sceneObjects;
	int selectedIndex = -1;

	void InitializeImGui();

	void UpdateUI();

	void UpdateObjectsHierarchyUI();
	void UpdateSettingsUI();
	void UpdateAddObjectUI();
	void UpdateObjectEditor();
};

#endif