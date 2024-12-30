#include "GUI.h"

void RaytracingGUI::UpdateObjectsHierarchyUI(bool& sceneChanged)
{
	ImGui::Begin("Objects");

	for (SceneObject& object : sceneObjects)
		ImGui::Button(object.name.c_str(), { 200, 20 });

	ImGui::End();
}

void RaytracingGUI::UpdateSettingsUI(Renderer& renderer, bool& sceneChanged)
{
	ImGui::Begin("Settings");

	bool settingsChanged = false;

	settingsChanged = settingsChanged || ImGui::InputInt("max bounces", &renderer.maxBounces);
	settingsChanged = settingsChanged || ImGui::InputInt("samples per pixel", &renderer.samplesPerPixel);
	settingsChanged = settingsChanged || ImGui::SliderFloat("perspective slope", &renderer.perspectiveSlope, 0.1f, 4.0f);
	settingsChanged = settingsChanged || ImGui::InputFloat("focal distance", &renderer.focalDistance);
	settingsChanged = settingsChanged || ImGui::InputFloat("focal blur", &renderer.focalBlur);

	if (settingsChanged)
	{
		renderer.UploadRaytraceSettings();
		sceneChanged = true;
	}

	ImGui::End();
}

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

void RaytracingGUI::UpdateUI(Renderer& renderer, Scene& scene, bool& sceneChanged)
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	
	UpdateObjectsHierarchyUI(sceneChanged);
	UpdateSettingsUI(renderer, sceneChanged);

	// Rendering
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
