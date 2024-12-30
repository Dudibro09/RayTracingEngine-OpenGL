#include "Objects.h"

void BoundingBox::GrowToInclude(const glm::vec3& point)
{
	min.x = std::min(point.x, min.x);
	min.y = std::min(point.y, min.y);
	min.z = std::min(point.z, min.z);

	max.x = std::max(point.x, max.x);
	max.y = std::max(point.y, max.y);
	max.z = std::max(point.z, max.z);
}

void BoundingBox::GrowToInclude(const Triangle& triangle)
{
	GrowToInclude(triangle.p[0]);
	GrowToInclude(triangle.p[1]);
	GrowToInclude(triangle.p[2]);
}

void Mesh::RecursiveBoundingBox(int boundingBoxIndex, std::vector<int>& triangleIndices)
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

		RecursiveBoundingBox(boundingBoxes[boundingBoxIndex].boundingBoxBIndex, triangleIndicesB);

		return;
	}

	// Add box a to list
	boundingBoxes[boundingBoxIndex].boundingBoxAIndex = boundingBoxes.size();
	boundingBoxes.push_back(boxA);
	RecursiveBoundingBox(boundingBoxes[boundingBoxIndex].boundingBoxAIndex, triangleIndicesA);

	// Add box b to list
	boundingBoxes[boundingBoxIndex].boundingBoxBIndex = boundingBoxes.size();
	boundingBoxes.push_back(boxB);
	RecursiveBoundingBox(boundingBoxes[boundingBoxIndex].boundingBoxBIndex, triangleIndicesB);
}

void Mesh::UpdateTransformMatrix()
{
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 rotationMatrix = glm::yawPitchRoll(rotation.y, rotation.x, rotation.z);
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

	// 4. Combine them to form the Local-to-World matrix
	localToWorldMatrix = translationMatrix * rotationMatrix * scaleMatrix;

	// 5. Invert the Local-to-World matrix to get the ModelWorld-to-Local matrix
	modelWorldToLocalMatrix = glm::inverse(localToWorldMatrix);
}

bool Mesh::LoadFromObjectFile(const char* file)
{
	triangles.clear();

	std::ifstream in(file, std::ios::binary);
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

			vertices.push_back(position);
		} break;
		case 'f': {
			if (firstTriangle)
			{
				firstTriangle = false;
				// Pre-reserve memory for the triangles to speed up loading
				triangles.reserve(triangles.size() + vertices.size() / 3);
			}

			int i0, i1, i2;
			char junk;

			ss >> junk >> i0 >> i1 >> i2;
			triangles.push_back({{ glm::vec4(vertices[i0 - 1], 1.0f), glm::vec4(vertices[i1 - 1], 1.0f), glm::vec4(vertices[i2 - 1], 1.0f) }});
		} break;
		default: break;
		}
	}

	return true;
}

void Mesh::UpdateBoundingBoxes()
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

	RecursiveBoundingBox(0, boxTriangles);
}