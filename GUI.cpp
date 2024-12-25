#include "GUI.h"

RaytracingGUI::RaytracingGUI(GLFWwindow* window) :
	io(ImGui::GetIO()),
	window(window)
{
	// Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     
	// Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; 

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	// Set the OpenGL version
	ImGui_ImplOpenGL3_Init("#version 460");
}

void RaytracingGUI::UpdateFrame()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	{
		ImGui::Begin("Objects");

		ImGui::Button("Object1", { 200, 20 });
		ImGui::Button("Object2", { 200, 20 });
		ImGui::Button("Object3", { 200, 20 });

		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
