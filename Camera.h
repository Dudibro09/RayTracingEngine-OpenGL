#pragma once
#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "Shader.h"

#define WORLD_FORWARD glm::vec3({ 0,0,1 })
#define WORLD_UP      glm::vec3({ 0,1,0 })
#define WORLD_RIGHT   glm::vec3({ 1,0,0 })

class Camera
{
private:
	bool firstClick = true;

public:

	glm::vec3 position;
	glm::vec3 rotation;

	float speed = 5.0f;
	float sensitivity = 3.0f;

	Camera();
	Camera(glm::vec3 position, glm::vec3 rotation);

	// Updates the camera position and rotation based on the user inputs
	void Inputs(GLFWwindow* window, const float& deltaTime, const unsigned int& width, const unsigned int& height, bool& changed);
};

#endif