#include "Scene.h"

void Scene::Initialize()
{
	glGenBuffers(1, &m_spheresSSBO);
	glGenBuffers(1, &m_meshesSSBO);
	glGenBuffers(1, &m_trianglesSSBO);
	glGenBuffers(1, &m_boundingBoxesSSBO);

	skybox.Initialize(GL_TEXTURE0);
}

void Scene::Uninitialize()
{
	glDeleteBuffers(1, &m_spheresSSBO);
	glDeleteBuffers(1, &m_meshesSSBO);
	glDeleteBuffers(1, &m_trianglesSSBO);
	glDeleteBuffers(1, &m_boundingBoxesSSBO);

	skybox.Delete();
}

void Scene::UpdateMesh(GLuint shaderID, const int& index)
{
	shaderReadyMeshes[index].localToWorldMatrix = meshes[index].localToWorldMatrix;
	shaderReadyMeshes[index].modelWorldToLocalMatrix = meshes[index].modelWorldToLocalMatrix;
	shaderReadyMeshes[index].material = meshes[index].material;

	glUseProgram(shaderID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_meshesSSBO);

	int offset = sizeof(ShaderReadyMesh) * index;
	ShaderReadyMesh* ptr = (ShaderReadyMesh*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, sizeof(ShaderReadyMesh), GL_MAP_WRITE_BIT);
	if (ptr)
	{
		(*ptr) = shaderReadyMeshes[index];
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}

	// Unbind the buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Scene::UpdateSphere(GLuint shaderID, const int& index)
{
	glUseProgram(shaderID);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_spheresSSBO);

	int offset = sizeof(Sphere) * index;
	Sphere* ptr = (Sphere*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, sizeof(Sphere), GL_MAP_WRITE_BIT);
	if (ptr)
	{
		(*ptr) = spheres[index];
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}

	// Unbind the buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Scene::AddMesh(const char* file)
{
	meshes.push_back(Mesh());
	meshes[meshes.size() - 1].LoadFromObjectFile(file);
	meshes[meshes.size() - 1].UpdateBoundingBoxes();
	meshes[meshes.size() - 1].UpdateTransformMatrix();
	meshes[meshes.size() - 1].material = Material({ 0.9f, 0.9f, 0.9f }, 0.8f, 0.0f, 0.5f, 1.0f, 1.5f, 1.0f);
}

void Scene::UpdateSSBO(GLuint shaderID)
{
	// Get all the scene data into a format our GPU can understand
	std::vector<Triangle> shaderReadyTriangles;
	std::vector<BoundingBox> shaderReadyBoundingBoxes;
	shaderReadyMeshes.clear();
	for (Mesh& mesh : meshes)
	{
		ShaderReadyMesh shaderReadyMesh;

		shaderReadyMesh.localToWorldMatrix = mesh.localToWorldMatrix;
		shaderReadyMesh.modelWorldToLocalMatrix = mesh.modelWorldToLocalMatrix;
		shaderReadyMesh.triangleIndex = shaderReadyTriangles.size();
		shaderReadyMesh.nTriangles = mesh.triangles.size();
		shaderReadyMesh.boundingBoxIndex = shaderReadyBoundingBoxes.size();
		shaderReadyMesh.nBoundingBoxes = mesh.boundingBoxes.size();
		shaderReadyMesh.material = mesh.material;

		shaderReadyMeshes.push_back(shaderReadyMesh);

		for (const Triangle& triangle : mesh.triangles)
		{
			shaderReadyTriangles.push_back(triangle);
		}
		for (const BoundingBox& boundingBox : mesh.boundingBoxes)
		{
			shaderReadyBoundingBoxes.push_back(boundingBox);
		}
	}

	std::cout << shaderReadyMeshes.size() << "meshes\n";
	std::cout << shaderReadyBoundingBoxes.size() << "bounding boxes\n";
	std::cout << shaderReadyTriangles.size() << "triangles\n";

	// Activate the shader so we can upload the scene data
	glUseProgram(shaderID);

	// SPHERES
	// Bind the shader storage buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_spheresSSBO);
	// Allocate storage for the SSBO
	glBufferData(GL_SHADER_STORAGE_BUFFER, spheres.size() * sizeof(Sphere), spheres.data(), GL_DYNAMIC_DRAW);
	// Update the triangle count uniform
	glUniform1ui(glGetUniformLocation(shaderID, "nSpheres"), spheres.size());

	// MESHES
	// Bind the shader storage buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_meshesSSBO);
	// Allocate storage for the SSBO
	glBufferData(GL_SHADER_STORAGE_BUFFER, shaderReadyMeshes.size() * sizeof(ShaderReadyMesh), shaderReadyMeshes.data(), GL_DYNAMIC_DRAW);
	// Update the triangle count uniform
	glUniform1ui(glGetUniformLocation(shaderID, "nMeshes"), shaderReadyMeshes.size());

	// TRIANGLES
	// Bind the shader storage buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_trianglesSSBO);
	// Allocate storage for the SSBO
	glBufferData(GL_SHADER_STORAGE_BUFFER, shaderReadyTriangles.size() * sizeof(Triangle), shaderReadyTriangles.data(), GL_DYNAMIC_DRAW);
	// Update the triangle count uniform
	glUniform1ui(glGetUniformLocation(shaderID, "nTriangles"), shaderReadyTriangles.size());

	// BOUNDING BOXES
	// Bind the shader storage buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_boundingBoxesSSBO);
	// Allocate storage for the SSBO
	glBufferData(GL_SHADER_STORAGE_BUFFER, shaderReadyBoundingBoxes.size() * sizeof(BoundingBox), shaderReadyBoundingBoxes.data(), GL_DYNAMIC_DRAW);
	// Update the triangle count uniform
	glUniform1ui(glGetUniformLocation(shaderID, "nBoundingBoxes"), shaderReadyBoundingBoxes.size());

	// Unbind the shader storage buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// Bind each shader storage buffer to a unique binding port
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_spheresSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_meshesSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_trianglesSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_boundingBoxesSSBO);
}