#include "Renderer.h"

void Renderer::Initialize(int width, int height)
{
	// Load OpenGL
	gladLoadGL();

	// Set up the rect to draw on
	GLfloat screenCoords[] = { -1,-1, -1,1, 1,1,  -1,-1, 1,1, 1,-1 };
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Upload the data to the GPU
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenCoords), screenCoords, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, 0);
	glEnableVertexAttribArray(0);

	// The render framebuffer object, to capture the output from the raytrace shader
	glGenFramebuffers(1, &renderFBO);
	// Initialize the texture
	renderTexture.Initialize(GL_TEXTURE1);
	renderTexture.Resize(width, height);
	// Bind the texture to the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture.ID, 0);

	// The final render framebuffer object, to capture the final render from the average program
	glGenFramebuffers(1, &finalRenderFBO);
	// Initialize the texture
	finalRenderTexture.Initialize(GL_TEXTURE2);
	finalRenderTexture.Resize(width, height);
	// Bind the texture to the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, finalRenderFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalRenderTexture.ID, 0);

	// Compile the shaders
	m_raytraceShader.LoadFromFile("raytrace.vert", "raytrace.frag");
	m_averageShader.LoadFromFile("average.vert", "average.frag");
	m_drawTextureShader.LoadFromFile("drawtexture.vert", "drawtexture.frag");

	// Upload the accumilate textures to the shaders
	renderTexture.UploadToShader("renderTex", m_averageShader.ID, 1);
	finalRenderTexture.UploadToShader("finalRenderTex", m_averageShader.ID, 2);
	finalRenderTexture.UploadToShader("tex", m_drawTextureShader.ID, 2);

	// Set the viewport resolution
	SetViewportResolution(width, height);
}

void Renderer::Uninitialize()
{
	renderTexture.Delete();
	finalRenderTexture.Delete();

	glDeleteFramebuffers(1, &renderFBO);
	glDeleteFramebuffers(1, &finalRenderFBO);

	m_raytraceShader.Delete();
	m_averageShader.Delete();
	m_drawTextureShader.Delete();
}

void Renderer::SetViewportResolution(int width, int height)
{
	// Set the gl viewport resolution
	glViewport(0, 0, width, height);

	// Resize the accumilate textures
	renderTexture.Resize(width, height);
	finalRenderTexture.Resize(width, height);

	// Update the aspect ratio
	m_raytraceShader.Activate();
	glUniform1f(glGetUniformLocation(m_raytraceShader.ID, "aspectRatio"), (float)height / (float)width);
}

int Renderer::GetRenderShaderID()
{
	return m_raytraceShader.ID;
}

void Renderer::UploadObjects(Scene& scene)
{
	scene.UpdateSSBO(m_raytraceShader.ID);
	scene.skybox.UploadToShader("skybox", m_raytraceShader.ID, 0);
}

void Renderer::UploadRaytraceSettings()
{
	m_raytraceShader.Activate();

	glUniform1i(glGetUniformLocation(m_raytraceShader.ID, "maxBounces"), maxBounces);
	glUniform1i(glGetUniformLocation(m_raytraceShader.ID, "samplesPerPixel"), samplesPerPixel);
	glUniform1f(glGetUniformLocation(m_raytraceShader.ID, "perspectiveSlope"), perspectiveSlope);
	glUniform1f(glGetUniformLocation(m_raytraceShader.ID, "focalDistance"), focalDistance);
	glUniform1f(glGetUniformLocation(m_raytraceShader.ID, "focalBlur"), focalBlur);
	glUniform1f(glGetUniformLocation(m_raytraceShader.ID, "blur"), blur);
}

void Renderer::UploadCameraView(Scene& scene)
{
	glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), scene.camera.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), scene.camera.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 rotationMatrix = rotationY * rotationX;

	// Upload the camera view matrices
	m_raytraceShader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(m_raytraceShader.ID, "cameraRotation"), 1, GL_FALSE, glm::value_ptr(rotationMatrix));
	glUniform3f(glGetUniformLocation(m_raytraceShader.ID, "cameraPosition"), scene.camera.position.x, scene.camera.position.y, scene.camera.position.z);
}

std::vector<float> Renderer::GetCurrentFrame(int& width, int& height)
{
	std::vector<float> data;

	finalRenderTexture.GetTextureData(width, height, data);

	return data;
}

void Renderer::Render(Scene& scene, bool& viewChanged)
{
	if (viewChanged)
	{
		viewChanged = false;
		frame = 0;
	}

	// Activate the raytrace shader
	m_raytraceShader.Activate();
	// Bind the skybox texture
	scene.skybox.Bind();
	// Upload the current frame count
	glUniform1ui(glGetUniformLocation(m_raytraceShader.ID, "frame"), frame);

	if (!renderMode)
	{
		// Activate the framebuffer to draw to
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Start ray tracing
		glDrawArrays(GL_TRIANGLES, 0, 6);

		return;
	}

	// Activate the framebuffer to draw to
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
	// Start ray tracing
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Activate the average shader
	m_averageShader.Activate();
	// Set the frame count
	glUniform1ui(glGetUniformLocation(m_averageShader.ID, "frame"), frame);
	// Bind the final render texture and the render texture
	finalRenderTexture.Bind();
	renderTexture.Bind();
	// Activate the framebuffer to draw to
	glBindFramebuffer(GL_FRAMEBUFFER, finalRenderFBO);
	// Calculate the average
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// Activate the draw texture shader
	m_drawTextureShader.Activate();
	// Bind the final render texture
	finalRenderTexture.Bind();
	// Bind the default framebuffer, the actual window
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Render the framebuffer texture to screen
	glDrawArrays(GL_TRIANGLES, 0, 6);

	frame++;
}