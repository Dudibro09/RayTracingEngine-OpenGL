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
	renderer.UploadRaytraceSettings(12, 1, 1.3f, 1.0f, 0.0f, 1.0f / (float)m_windowHeight * 1.1f);

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

	renderer.SetViewportResolution(newWidth, newHeight);
	m_viewChanged = true;
}

void App::LoadScene()
{
	scene.skybox.LoadFromFile("skyboxes/Powder blue sky.jpg");
	scene.camera.position = { 0.0f,5.0f,-10.0f };
	scene.camera.rotation = { 0.6f,0.0f,0.0f };

	//scene.spheres.push_back({ {0,4,0}, 0.5, { { 1.0f, 1.0f, 1.0f }, 1.0f, 20.0f, 0, 0, 1.0f } });

	/* Laser light caustics
	scene.spheres.push_back({ glm::vec3(0,-1,0), 1.0f, Material(glm::vec3(0.9f, 0.9f, 0.9f), 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f) });
	//scene.spheres.push_back({ glm::vec3(0,1,0), 1.0f, Material(glm::vec3(0.9f, 0.9f, 0.9f), 0.0f, 0.0f, 0.0f, 0.1f, 1.5f, 0.0f) });
	//scene.LoadMesh("cube.obj", { 0.0f,14.0f,12.0f }, { 1.0f,0.0f,0.0f }, glm::vec3(10.0f), Material(glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, 1.0f, 0.2f, 0.0f, 1.0f, 1.0f));
	//*/

	//* Glass dragon test (5 million triangles)
	//scene.spheres.push_back({ {0,-100,0}, 100, { { 1.0f, 1.0f, 1.0f }, 1.0f, 0, 0, 0, 1.0f } });
	scene.LoadMesh("objects/drag0.obj", { 0.0f,1.0f,0.0f }, { 0.0f,0.0f,0.0f }, glm::vec3(1.0f), {{0.1f, 0.9f, 0.5f}, 0.7f, 0.0f, 0.3f, 0.0f, 1.0f, 1.0f});
	//*/

	/* All meshes and spheres material test
	scene.LoadMesh("cube.obj", { 0.0f,5.0f,0.0f }, { 0.0f,0.0f,0.0f }, glm::vec3(1.0f), { { 0.9f,0.1f,0.1f }, 0.1f, 0, 0, 1.0f, 1.0f });
	scene.LoadMesh("cube.obj", { 3.0f,5.0f,0.0f }, { 0.0f,0.0f,0.0f }, glm::vec3(1.0f), { { 0.9f,0.9f,0.1f }, 0.9f, 0, 0, 1.0, 1.0f });
	scene.LoadMesh("cube.obj", { 6.0f,5.0f,0.0f }, { 0.0f,0.0f,0.0f }, glm::vec3(1.0f), { { 0.9f,0.9f,0.9f }, 0.0f, 0, 0, 1.5f, 0.1f });
	scene.LoadMesh("cube.obj", { 9.0f,5.0f,0.0f }, { 0.0f,0.0f,0.0f }, glm::vec3(1.0f), { { 0.1f,0.9f,0.9f }, 0.0f, 0, 1, 1.0f, 0.1f });
	scene.LoadMesh("cube.obj", { 12.0f,5.0f,0.0f}, { 0.0f,0.0f,0.0f }, glm::vec3(1.0f), { { 0.1f,0.9f,0.9f }, 0.1f, 0, 0, 1.5f, 0.1f });

	scene.spheres.push_back({ {0,2,0}, 1, { { 0.9f,0.1f,0.1f }, 0.1f, 0, 0, 1.0f, 1.0f } });
	scene.spheres.push_back({ {3,2,0}, 1, { { 0.9f,0.9f,0.1f }, 0.9f, 0, 0, 1.0, 1.0f } });
	scene.spheres.push_back({ {6,2,0}, 1, { { 0.9f,0.9f,0.9f }, 0.0f, 0, 0, 1.5f, 0.1f } });
	scene.spheres.push_back({ {6,2,3}, 1, { { 0.9f,0.9f,0.9f }, 0.0f, 0, 0, 1.5f, 0.1f } });
	scene.spheres.push_back({ {9,2,0}, 1, { { 0.1f,0.9f,0.9f }, 0.0f, 0, 1, 1.0f, 0.1f } });
	scene.spheres.push_back({ {12,2,0},1, { { 0.1f,0.9f,0.9f }, 0.1f, 0, 0, 1.125f, 0.1f } });
	//*/

	// Precompute the bounding boxes for the triangles
	scene.UpdateBoundingBox();
	// Upload the objects to the shader
	renderer.UploadScene(scene);
}

void App::ScreenShot()
{
	std::cout << "Saved frame!\n";

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

	// Save the frame
	stbi_flip_vertically_on_write(1);
	stbi_write_jpg(("renders/render-" + oss.str() + ".jpg").c_str(), texWidth, texHeight, 3, frame.data(), 100);

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
		scene.camera.Inputs(m_window, m_deltaTime, m_windowWidth, m_windowHeight, m_viewChanged);
		renderer.UploadCameraView(scene);

		// Start rendering/raytracing
		renderer.Render(scene, m_viewChanged);
		// Update the UI
		UI.UpdateFrame();

		// Take a screepshot of the current render if the user has pressed "P"
		if (glfwGetKey(m_window, GLFW_KEY_P))
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