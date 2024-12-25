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