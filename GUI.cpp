#include "GUI.h"

void RaytracingGUI::UpdateObjectsHierarchyUI(bool& sceneChanged)
{
	ImGui::Begin("Scene");

	ImGui::Button("Change skybox");

	if (ImGui::Button("Add Object"))
	{
		addObjectWindowOpen = true;
	}


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
	if (ImGui::Checkbox("render mode", &renderer.renderMode))
	{
		glfwMakeContextCurrent(window);
		glfwSwapInterval((int)(!renderer.renderMode));
		settingsChanged = true;
	}

	if (settingsChanged)
	{
		renderer.UploadRaytraceSettings();
		sceneChanged = true;
	}

	ImGui::End();
}

void RaytracingGUI::UpdateAddObjectUI(Scene& scene, Renderer& renderer, bool &sceneChanged)
{
	if (!addObjectWindowOpen) return;

	ImGui::Begin("AddObject");

	if (ImGui::Button("Sphere"))
	{
		scene.spheres.push_back({ { 0,0,0 }, 1.0f, Material({0.9f, 0.9f, 0.9f}, 0.7f, 0,0,0,0,1)});
		renderer.UploadObjects(scene);
		addObjectWindowOpen = false;
		
		sceneObjects.push_back({ "Sphere", ObjectType::TYPE_SPHERE, (int)scene.spheres.size() - 1 });
	}

	ImGui::End();
}

RaytracingGUI::RaytracingGUI(GLFWwindow* window) :
	io(ImGui::GetIO()),
	window(window)
{
	glfwMakeContextCurrent(window);

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
	UpdateAddObjectUI(scene, renderer, sceneChanged);

	// Rendering
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
