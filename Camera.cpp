#include "Camera.h"

Camera::Camera() :
	position(0),
	rotation(0)
{}

Camera::Camera(glm::vec3 position, glm::vec3 rotation) :
	position(position),
	rotation(rotation)
{}

void Camera::Inputs(GLFWwindow* window, const float& deltaTime, const unsigned int& width, const unsigned int& height, bool& changed)
{
	glm::vec3 forward = glm::rotateY(WORLD_FORWARD, rotation.y);
	glm::vec3 right = glm::rotateY(WORLD_RIGHT, rotation.y);
	glm::vec3 up = WORLD_UP;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		position += forward * deltaTime * speed;
		changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		position -= forward * deltaTime * speed;
		changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		position += right * deltaTime * speed;
		changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		position -= right * deltaTime * speed;
		changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		position += deltaTime * speed * up;
		changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		position -= deltaTime * speed * up;
		changed = true;
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		rotation.y += deltaTime * sensitivity;
		if (rotation.y > glm::radians(360.0f)) rotation.y -= glm::radians(360.0f);
		changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		rotation.y -= deltaTime * sensitivity;
		if (rotation.y < glm::radians(0.0f)) rotation.y += glm::radians(360.0f);
		changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		rotation.x += deltaTime * sensitivity;
		if (rotation.x > glm::radians(90.0f)) rotation.x = glm::radians(90.0f);
		changed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		rotation.x -= deltaTime * sensitivity;
		if (rotation.x < glm::radians(-90.0f)) rotation.x = glm::radians(-90.0f);
		changed = true;
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		if (firstClick)
		{
			glfwSetCursorPos(window, width / 2, height / 2);
			firstClick = false;
		}

		double mouseX;
		double mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		rotation.x += sensitivity * (float)(mouseY - (height / 2)) / height;
		rotation.y += sensitivity * (float)(mouseX - (width / 2)) / height;

		if (rotation.y > glm::radians(360.0f)) rotation.y -= glm::radians(360.0f);
		if (rotation.y < glm::radians(360.0f)) rotation.y += glm::radians(360.0f);

		if (rotation.x > glm::radians(90.0f)) rotation.x = glm::radians(90.0f);
		if (rotation.x < glm::radians(-90.0f)) rotation.x = glm::radians(-90.0f);

		glfwSetCursorPos(window, width / 2, height / 2);

		changed = true;
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		firstClick = true;
	}
}