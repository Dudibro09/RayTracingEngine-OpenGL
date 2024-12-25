#pragma once
#ifndef GUI_H
#define GUI_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

class RaytracingGUI
{
	ImGuiIO& io;
	GLFWwindow* window;

	bool demo_window = true;

public:
	RaytracingGUI(GLFWwindow* window);

	void UpdateFrame();
};

#endif