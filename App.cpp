#include "App.h"

App::App(int width, int height, const std::string& title) :
	m_windowWidth(width), 
	m_windowHeight(height),
	m_windowTitle(title)
{
	// Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a new window
	m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_windowTitle.c_str(), NULL, NULL);
	if (m_window == NULL)
	{
		glfwTerminate();
		throw std::string("Failed to create GLFW window\n");
		return;
	}
	glfwMakeContextCurrent(m_window);
	// Associate this App instance with the window
	glfwSetWindowUserPointer(m_window, this);
	// Set window callback functions
	glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
	glfwSetDropCallback(m_window, drop_callback);
	// v-sync
	glfwSwapInterval(1);

	// Initialize the renderer class
	m_renderer.Initialize(m_windowWidth, m_windowHeight);
	// Set the raytracing settings
	m_renderer.UploadRaytraceSettings();

	// Initialize the scene class
	m_scene.Initialize();
	// Load the scene
	LoadScene();

	// Initialize Dear ImGui
	InitializeImGui();
}

App::~App()
{
	m_renderer.Uninitialize();

	glfwDestroyWindow(m_window);
	glfwTerminate();
}

int App::Start()
{
	while (!glfwWindowShouldClose(m_window))
	{
		// Update the camera position based on controls
		m_scene.camera.Inputs(m_window, m_deltaTime, m_windowWidth, m_windowHeight, m_sceneChanged);
		m_renderer.UploadCameraView(m_scene);

		// Start rendering/raytracing
		m_renderer.Render(m_scene, m_sceneChanged);
		// Update the UI
		UpdateUI();

		// Take a screenshot of the current render if the user has pressed "P"
		if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) && glfwGetKey(m_window, GLFW_KEY_P))
		{
			ScreenShot();
		}

		// Update window
		glfwSwapBuffers(m_window);
		glfwPollEvents();
		UpdateTimer();
	}

	return 0;
}

void App::UpdateTimer()
{
	m_crntTime = glfwGetTime();
	m_deltaTime = m_crntTime - m_prevTime;
	m_fTheta += m_deltaTime;
	m_prevTime = m_crntTime;
	m_frameCount++;

	// Update the window title every 200 milliseconds to display the FPS
	if (m_fTheta >= 0.2)
	{
		std::string FPS = std::to_string((int)(1.0 / m_fTheta * m_frameCount));
		std::string ms = std::to_string((m_fTheta / m_frameCount * 1000.0));
		std::string newTitle = "OpenGL - " + FPS + " FPS - " + ms + "ms";
		glfwSetWindowTitle(m_window, newTitle.c_str());

		m_fTheta = 0.0;
		m_frameCount = 0;
	}
}

void App::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
	if (app) 
	{
		app->HandleWindowResize(width, height);
	}
	else
	{
		std::cout << "Failed to get the pointer\n";
	}
}

void App::HandleWindowResize(int newWidth, int newHeight)
{
	m_windowWidth = newWidth;
	m_windowHeight = newHeight;

	// Update the anti-aliasing
	m_renderer.blur = 1.1f / (float)m_windowHeight;
	// Set the new viewport resolution
	m_renderer.SetViewportResolution(newWidth, newHeight);
	// Scene changed
	m_sceneChanged = true;
}

void App::drop_callback(GLFWwindow* window, int count, const char** paths) 
{
	App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
	if (app)
	{
		app->HandleFileDragDrop(count, paths);
	}
	else
	{
		std::cout << "Failed to get the pointer\n";
	}	
}

void App::HandleFileDragDrop(int count, const char** paths)
{
	for (int i = 0; i < count; i++)
	{
		std::string path = paths[i];

		size_t extentionPosition = path.find_last_of('.');
		if (extentionPosition == std::string::npos)
		{
			std::cout << "Invalid path: " << path << "\n";
			continue;
		}

		std::string extention = path.substr(extentionPosition);

		if (extention == ".obj")
		{
			m_scene.AddMesh(path.c_str());
			m_renderer.UploadObjects(m_scene);
			m_sceneChanged = true;
		}
		else if (extention == ".jpg")
		{
			m_scene.skybox.LoadFromFile(path.c_str());
			m_renderer.UploadObjects(m_scene);
			m_sceneChanged = true;
		}
		else
		{
			std::cout << "Unknown file extention '" << extention << "' from '" << path << "'\n";
		}
	}
}

void App::ScreenShot()
{
	int texWidth, texHeight;
	std::vector<float> rawData = m_renderer.GetCurrentFrame(texWidth, texHeight);

	std::vector<unsigned char> frame(texWidth * texHeight * 3, 0);

	for (int i = 0; i < texWidth * texHeight * 3; i++)
	{
		frame[i] = (unsigned char)std::max(0.0f, std::min(255.0f, rawData[i] * 255.0f));
	}

	// Get the current time point
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
	std::ostringstream oss;
	oss << std::put_time(std::gmtime(&time), "%Y%m%d_%H%M%S") << '_' << std::setw(3) << std::setfill('0') << ms.count();

	std::string fileLocation = "renders/render-" + oss.str() + ".jpg";

	// Save the frame
	stbi_flip_vertically_on_write(1);
	stbi_write_jpg(fileLocation.c_str(), texWidth, texHeight, 3, frame.data(), 100);

	std::cout << "Saved frame: " << fileLocation << "\n";

	Sleep(500);
}

void App::LoadScene()
{
	m_scene.skybox.LoadFromFile("skyboxes/Powder blue sky.jpg");
	m_scene.camera.position = { 0.0f,5.0f,-10.0f };
	m_scene.camera.rotation = { 0.6f,0.0f,0.0f };

	// Upload the objects to the shader
	m_renderer.UploadObjects(m_scene);
}

void App::InitializeImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	m_io = &ImGui::GetIO();

	// Enable Keyboard Controls
	m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	// Enable Gamepad Controls
	m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	// Set the OpenGL version
	ImGui_ImplOpenGL3_Init("#version 460");
}

void App::UpdateUI()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	UpdateObjectsHierarchyUI();
	UpdateSettingsUI();
	UpdateAddObjectUI();
	UpdateObjectEditor();

	// Rendering
	ImGui::Render();
	int display_w, display_h;
	glfwGetFramebufferSize(m_window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void App::UpdateObjectsHierarchyUI()
{
	ImGui::Begin("Scene");

	if (ImGui::Button("Add Object"))
	{
		m_isAddObjectWindowOpen = true;
	}

	for (int i = 0; i < sceneObjects.size(); i++)
	{
		SceneObject& object = sceneObjects[i];

		if (ImGui::Button(object.name.c_str(), { 200, 20 }))
		{
			selectedIndex = i;
		}
	}

	ImGui::End();
}

void App::UpdateSettingsUI()
{
	ImGui::Begin("Settings");

	bool settingsChanged = false;

	if (ImGui::InputInt("max bounces", &m_renderer.maxBounces)) settingsChanged = true;
	if (ImGui::InputInt("samples per pixel", &m_renderer.samplesPerPixel)) settingsChanged = true;
	if (ImGui::SliderFloat("perspective slope", &m_renderer.perspectiveSlope, 0.1f, 4.0f)) settingsChanged = true;
	if (ImGui::InputFloat("focal distance", &m_renderer.focalDistance)) settingsChanged = true;
	if (ImGui::InputFloat("focal blur", &m_renderer.focalBlur)) settingsChanged = true;
	if (ImGui::Checkbox("render mode", &m_renderer.renderMode))
	{
		glfwMakeContextCurrent(m_window);
		glfwSwapInterval((int)(!m_renderer.renderMode));
		settingsChanged = true;
	}

	if (settingsChanged)
	{
		m_renderer.UploadRaytraceSettings();
		m_sceneChanged = true;
	}

	ImGui::End();
}

void App::UpdateAddObjectUI()
{
	if (!m_isAddObjectWindowOpen) return;

	ImGui::Begin("AddObject");

	ImGui::Text("Shapes");
	if (ImGui::Button("Sphere", { 220, 20 }))
	{
		m_scene.spheres.push_back({ { 0,0,0 }, 1.0f, Material({0.9f, 0.9f, 0.9f}, 0.8f, 0.0f, 0.5f, 1.0f, 1.5f, 1.0f ) });
		m_renderer.UploadObjects(m_scene);
		m_isAddObjectWindowOpen = false;

		std::string name = ("Sphere" + std::to_string(m_scene.spheres.size()));
		sceneObjects.push_back({ name, ObjectType::TYPE_SPHERE, (int)m_scene.spheres.size() - 1 });
	}

	ImGui::Text("\nMeshes");

	std::string location = "objects";

	for (const auto& entry : std::filesystem::directory_iterator(location))
	{
		std::string filename = entry.path().filename().string();

		if (ImGui::Button(filename.c_str(), { 220, 20 }))
		{
			m_scene.AddMesh((location + "/" + filename).c_str());
			m_scene.UpdateSSBO(m_renderer.GetRenderShaderID());
			m_isAddObjectWindowOpen = false;

			std::string name = ("Mesh" + std::to_string(m_scene.meshes.size()));
			sceneObjects.push_back({ name, ObjectType::TYPE_MESH, (int)m_scene.meshes.size() - 1 });
		}
	}

	ImGui::End();
}

void App::UpdateObjectEditor()
{
	ImGui::Begin("ObjectEditor");

	if (selectedIndex == -1)
	{
		ImGui::End();
		return;
	}

	SceneObject& objectRefrence = sceneObjects[selectedIndex];

	glm::vec3* position;
	glm::vec3* rotation;
	glm::vec3* scale;
	float* radius;
	Material* material;

	switch (objectRefrence.type)
	{
	case TYPE_SPHERE: {
		Sphere& sphere = m_scene.spheres[objectRefrence.index];
		position = &sphere.position;
		rotation = nullptr;
		scale = nullptr;
		radius = &sphere.radius;
		material = &sphere.material;
		break;
	}
	case TYPE_MESH: {
		Mesh& mesh = m_scene.meshes[objectRefrence.index];
		position = &mesh.position;
		rotation = &mesh.rotation;
		scale = &mesh.scale;
		radius = nullptr;
		material = &mesh.material;
		break;
	}
	default:
		std::cout << "Trying to get object info of an unknown object type\n";
		return;
	}

	// Transform
	bool objectChanged = false;

	ImGui::Text("\Transform");

	if (position != nullptr)
	{
		if (ImGui::InputFloat3("position", (float*)position)) objectChanged = true;
	}

	if (rotation != nullptr)
	{
		if (ImGui::InputFloat3("rotation", (float*)rotation)) objectChanged = true;
	}

	if (scale != nullptr)
	{
		if (ImGui::InputFloat3("scale", (float*)scale)) objectChanged = true;
	}

	if (radius != nullptr)
	{
		if (ImGui::InputFloat("radius", (float*)radius)) objectChanged = true;
	}

	// Material
	ImGui::Text("\nMaterial");

	if (ImGui::InputFloat3("material", (float*)&material->color))
	{
		material->absorbColor = glm::vec3(1.0f) - material->color;
		material->emissionColor = material->color;
		objectChanged = true;
	}

	if (ImGui::SliderFloat("roughness", (float*)&material->roughness, 0.0f, 1.0f))
	{
		objectChanged = true;
	}

	if (ImGui::InputFloat("emission strength", (float*)&material->emissionStrength))
	{
		objectChanged = true;
	}

	if (ImGui::SliderFloat("emission scattering index", (float*)&material->emissionScatteringIndex, 0.0f, 1.0f))
	{
		objectChanged = true;
	}

	if (ImGui::InputFloat("absorbsion strength", (float*)&material->absorbsionStrength))
	{
		objectChanged = true;
	}

	if (ImGui::InputFloat("refractive index", (float*)&material->refractiveIndex))
	{
		objectChanged = true;
	}

	if (ImGui::SliderFloat("reflective index", (float*)&material->reflectiveIndex, 0.0f, 1.0f))
	{
		objectChanged = true;
	}

	if (objectChanged)
	{
		switch (objectRefrence.type)
		{
		case TYPE_SPHERE:
			m_scene.UpdateSphere(m_renderer.GetRenderShaderID(), objectRefrence.index);
			break;
		case TYPE_MESH:
			m_scene.meshes[objectRefrence.index].UpdateTransformMatrix();
			m_scene.UpdateMesh(m_renderer.GetRenderShaderID(), objectRefrence.index);
			break;
		default:
			std::cout << "Trying to update an unknown object type\n";
			return;
		}

		m_sceneChanged = true;
	}

	ImGui::End();
}