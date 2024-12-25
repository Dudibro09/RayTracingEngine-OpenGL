#include "Scene.h"

void Scene::RecursiveBoundingBox(int boundingBoxIndex, std::vector<int>& triangleIndices, int depth)
{
	if (triangleIndices.size() == 1)
	{
		boundingBoxes[boundingBoxIndex].triangleIndex = triangleIndices[0];
		return;
	}
	else if (triangleIndices.size() == 0)
	{
		// No triangles provided
		return;
	}

	// Get the split axis
	int axisIndex = -1;

	BoundingBox boxA;
	std::vector<int> triangleIndicesA;
	BoundingBox boxB;
	std::vector<int> triangleIndicesB;

	if (axisIndex == -1)
	{
		glm::vec3 scale = boundingBoxes[boundingBoxIndex].max - boundingBoxes[boundingBoxIndex].min;
		if (scale.x > scale.y && scale.x > scale.z) axisIndex = 0;
		else if (scale.y > scale.x && scale.y > scale.z) axisIndex = 1;
		else axisIndex = 2;
	}
	else
	{
		axisIndex += 1;
		if (axisIndex > 2) axisIndex = 0;
	}

	float fCenterPos = (boundingBoxes[boundingBoxIndex].min[axisIndex] + boundingBoxes[boundingBoxIndex].max[axisIndex]) * 0.5f;

	for (int i : triangleIndices)
	{
		Triangle triangle = triangles[i];

		//Vector3d trianglePosition = triangle->vertices[0];
		glm::vec3 trianglePosition = (triangle.p[0] + triangle.p[1] + triangle.p[2]) / 3.0f;
		float fTriPos = trianglePosition[axisIndex];

		if (fTriPos < fCenterPos) // The triangle is in the first bounding box
		{
			boxA.GrowToInclude(triangle);
			triangleIndicesA.push_back(i);
		}
		else // The triangle is in the second bounding box
		{
			boxB.GrowToInclude(triangle);
			triangleIndicesB.push_back(i);
		}
	}

	if (triangleIndicesA.size() == 0 || triangleIndicesB.size() == 0)
	{
		// Just distribute them
		boxA = BoundingBox();
		triangleIndicesA.clear();

		// Finish the bounding box
		boxA.GrowToInclude(triangles[triangleIndices[0]]);
		triangleIndicesA.push_back(triangleIndices[0]);
		boxA.triangleIndex = triangleIndices[0];

		// Add box a to list
		boundingBoxes[boundingBoxIndex].boundingBoxAIndex = boundingBoxes.size();
		boundingBoxes.push_back(boxA);

		boxB = BoundingBox();
		triangleIndicesB.clear();

		for (int i = 1; i < triangleIndices.size(); i++)
		{
			int index = triangleIndices[i];
			boxB.GrowToInclude(triangles[index]);
			triangleIndicesB.push_back(index);
		}

		// Add box b to list
		boundingBoxes[boundingBoxIndex].boundingBoxBIndex = boundingBoxes.size();
		boundingBoxes.push_back(boxB);

		RecursiveBoundingBox(boundingBoxes[boundingBoxIndex].boundingBoxBIndex, triangleIndicesB, depth + 1);

		return;
	}

	// Add box a to list
	boundingBoxes[boundingBoxIndex].boundingBoxAIndex = boundingBoxes.size();
	boundingBoxes.push_back(boxA);
	RecursiveBoundingBox(boundingBoxes[boundingBoxIndex].boundingBoxAIndex, triangleIndicesA, depth + 1);

	// Add box b to list
	boundingBoxes[boundingBoxIndex].boundingBoxBIndex = boundingBoxes.size();
	boundingBoxes.push_back(boxB);
	RecursiveBoundingBox(boundingBoxes[boundingBoxIndex].boundingBoxBIndex, triangleIndicesB, depth + 1);
}

void Scene::Initialize()
{
	glGenBuffers(1, &trianglesSSBO);
	glGenBuffers(1, &spheresSSBO);
	glGenBuffers(1, &boundingBoxesSSBO);
	glGenBuffers(1, &sortedTriangleIndicesSSBO);

	skybox.Initialize(GL_TEXTURE0);
}

void Scene::Uninitialize()
{
	glDeleteBuffers(1, &trianglesSSBO);
	glDeleteBuffers(1, &spheresSSBO);
	glDeleteBuffers(1, &boundingBoxesSSBO);
	glDeleteBuffers(1, &sortedTriangleIndicesSSBO);

	skybox.Delete();
}

bool Scene::LoadMesh(const char* path, glm::vec3 offset, glm::vec3 rotation, glm::vec3 scale, const Material& material)
{
	std::ifstream in(path, std::ios::binary);
	if (!in.is_open())
	{
		return false;
	}

	std::vector<glm::vec3> vertices;
	bool firstTriangle = true;

	while (!in.eof())
	{
		std::string line;
		std::getline(in, line);

		std::stringstream ss(line);

		switch (line[0])
		{
		case 'v': {
			glm::vec3 position;
			char junk;

			ss >> junk >> position.x >> position.y >> position.z;

			// Transform the point
			glm::mat4 matRotation = 
				glm::rotate(rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::rotate(rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) * 
				glm::rotate(rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));


			position.x *= scale.x;
			position.y *= scale.y;
			position.z *= scale.z;
			position = glm::vec4(position, 1.0f) * matRotation;
			position += offset;

			vertices.push_back(position);
		} break;
		case 'f': {
			if (firstTriangle)
			{
				firstTriangle = false;
				triangles.reserve(triangles.size() + vertices.size() / 3);
			}

			int i0, i1, i2;
			char junk;

			ss >> junk >> i0 >> i1 >> i2;

			triangles.push_back({
				{ glm::vec4(vertices[i0 - 1], 1.0f), glm::vec4(vertices[i1 - 1], 1.0f), glm::vec4(vertices[i2 - 1], 1.0f) },
				material
				});
		} break;
		default: break;
		}
	}

	std::cout << "Loaded " << triangles.size() << " triangles\n";

	return true;
}

void Scene::UpdateBoundingBox()
{
	if (triangles.size() == 0) return;

	boundingBoxes.clear();
	boundingBoxes.push_back(BoundingBox());

	std::vector<int> boxTriangles;
	for (int i = 0; i < triangles.size(); i++)
	{
		boundingBoxes[0].GrowToInclude(triangles[i]);
		boxTriangles.push_back(i);
	}

	RecursiveBoundingBox(0, boxTriangles, 1);
}

void Scene::UpdateSSBO(GLuint shaderID)
{
	glUseProgram(shaderID);

	// SPHERES
	// Bind the shader storage buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, spheresSSBO);
	// Allocate storage for the SSBO
	glBufferData(GL_SHADER_STORAGE_BUFFER, spheres.size() * sizeof(Sphere), spheres.data(), GL_DYNAMIC_DRAW);
	// Update the triangle count uniform
	glUniform1ui(glGetUniformLocation(shaderID, "nSpheres"), spheres.size());

	int size = triangles.size();
	//std::cout << size << "\n";

	// TRIANGLES
	// Bind the shader storage buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, trianglesSSBO);
	// Allocate storage for the SSBO
	glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(Triangle), triangles.data(), GL_DYNAMIC_DRAW);
	// Update the triangle count uniform
	glUniform1ui(glGetUniformLocation(shaderID, "nTriangles"), size);

	// BOUNDING BOXES
	// Bind the shader storage buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, boundingBoxesSSBO);
	// Allocate storage for the SSBO
	glBufferData(GL_SHADER_STORAGE_BUFFER, boundingBoxes.size() * sizeof(BoundingBox), boundingBoxes.data(), GL_DYNAMIC_DRAW);
	// Update the triangle count uniform
	glUniform1ui(glGetUniformLocation(shaderID, "nBoundingBoxes"), boundingBoxes.size());

	// SORTED TRIANGLE INDICES
	// Bind the shader storage buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, sortedTriangleIndicesSSBO);
	// Allocate storage for the SSBO
	glBufferData(GL_SHADER_STORAGE_BUFFER, sortedTriangleIndices.size() * sizeof(int), sortedTriangleIndices.data(), GL_DYNAMIC_DRAW);

	// Unbind the shader storage buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Bind each shader storage buffer to a unique binding port
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, spheresSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, trianglesSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, boundingBoxesSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, sortedTriangleIndicesSSBO);
}