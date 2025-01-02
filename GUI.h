#pragma once
#ifndef GUI_H
#define GUI_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Scene.h"
#include "Renderer.h"

enum ObjectType : unsigned int
{
	TYPE_SPHERE,
	TYPE_MESH
};

struct SceneObject
{
	std::string name;
	ObjectType type;
	int index;
};

class RaytracingGUI
{
	ImGuiIO& io;
	GLFWwindow* window;


	std::vector<SceneObject> sceneObjects;

	void UpdateObjectsHierarchyUI(bool& sceneChanged);
	void UpdateSettingsUI(Renderer& renderer, bool& sceneChanged);
	void UpdateAddObjectUI(Scene& scene, Renderer& renderer, bool& sceneChanged);
	void UpdateObjectEditor(Scene& scene, Renderer& renderer, bool& sceneChanged);

	bool addObjectWindowOpen = false;

public:
	RaytracingGUI(GLFWwindow* window);

	void UpdateUI(Renderer& renderer, Scene& scene, bool &sceneChanged);
};

#endif