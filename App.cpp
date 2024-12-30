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
	// Set the window resizing callback function
	glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
	// Disable v-sync
	glfwSwapInterval(0);

	// Initialize the renderer class
	renderer.Initialize(m_windowWidth, m_windowHeight);
	// Set the raytracing settings
	renderer.UploadRaytraceSettings();

	// Initialize the scene class
	scene.Initialize();
	// Load the scene
	LoadScene();
}

App::~App()
{
	renderer.Uninitialize();

	glfwDestroyWindow(m_window);
	glfwTerminate();
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
	renderer.blur = 1.1f / (float)m_windowHeight;
	// Set the new viewport resolution
	renderer.SetViewportResolution(newWidth, newHeight);
	// Scene changed
	sceneChanged = true;
}

void App::LoadScene()
{
	scene.skybox.LoadFromFile("skyboxes/Powder blue sky.jpg");
	scene.camera.position = { 0.0f,5.0f,-10.0f };
	scene.camera.rotation = { 0.6f,0.0f,0.0f };

	scene.meshes.push_back(Mesh());
	if (!scene.meshes[0].LoadFromObjectFile("objects/cube.obj")) std::cout << "Failed to load!\n";
	scene.meshes[0].UpdateBoundingBoxes();
	scene.meshes[0].material = Material({ 0,1,0 }, 0.1f, 0, 0, 0, 1, 1);
	scene.meshes[0].position = { 10,0,0 };
	scene.meshes[0].rotation = { 0,0.2f,0 };
	scene.meshes[0].scale = { 1,0.2f,1 };
	scene.meshes[0].UpdateTransformMatrix();

	scene.meshes.push_back(Mesh());
	if (!scene.meshes[1].LoadFromObjectFile("objects/drag4.obj")) std::cout << "Failed to load!\n";
	scene.meshes[1].UpdateBoundingBoxes();
	scene.meshes[1].material = Material({1,0,0}, 0.8f, 0, 0, 0, 1, 1);
	scene.meshes[1].UpdateTransformMatrix();

	scene.meshes.push_back(Mesh());
	if (!scene.meshes[2].LoadFromObjectFile("objects/cube.obj")) std::cout << "Failed to load!\n";
	scene.meshes[2].UpdateBoundingBoxes();
	scene.meshes[2].material = Material({ 0,1,0 }, 0.1f, 0, 0, 0, 1, 1);
	scene.meshes[2].position = { 7,0,0 };
	scene.meshes[2].rotation = { 0,0.0f,0 };
	scene.meshes[2].UpdateTransformMatrix();

	//scene.spheres.push_back({ { 0,0,0 }, 1.0f, Material({ 0,1,0 }, 0.1f, 0, 0, 0, 1, 1) });

	// Upload the objects to the shader
	renderer.UploadObjects(scene);
}

void App::ScreenShot()
{
	int texWidth, texHeight;
	std::vector<float> rawData = renderer.GetCurrentFrame(texWidth, texHeight);

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

int App::Start()
{
	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	RaytracingGUI UI(m_window);

	while (!glfwWindowShouldClose(m_window))
	{
		// Update the camera position based on controls
		scene.camera.Inputs(m_window, m_deltaTime, m_windowWidth, m_windowHeight, sceneChanged);
		renderer.UploadCameraView(scene);

		// Start rendering/raytracing
		renderer.Render(scene, sceneChanged);
		// Update the UI
		UI.UpdateUI(renderer, scene, sceneChanged);

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