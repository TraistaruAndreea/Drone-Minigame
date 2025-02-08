#pragma once

#include <string>

#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"


namespace object2D
{

	// Create square with given bottom left corner, length and color
	Mesh* CreateSquare(const std::string& name, glm::vec3 leftBottomCorner, float length, glm::vec3 color, bool fill = false);
	Mesh* CreateTrapezoid(const std::string& name, float width, float height, float length, glm::vec3 color, bool fill = false);
	Mesh* CreateCircle(const std::string& name, glm::vec3 center, float radius, glm::vec3 color, bool fill = false);
	Mesh* CreateRectangle(const std::string& name, glm::vec3 leftBottomCorner, float length, float width, glm::vec3 color, bool fill = false);
	Mesh* CreateGrowingDustCloud(const std::string& name, glm::vec3 center, float initialRadius, float growthRate, glm::vec3 color, int numSegments, float time);
	Mesh* CreateParallelepiped(const std::string& name, glm::vec3 corner, float lengthX, float lengthY, float lengthZ, glm::vec3 color);
	Mesh* CreateCube(const std::string& name, glm::vec3 corner, float sideLength, glm::vec3 color);
	Mesh* CreateCylinder(float height, float radius, const glm::vec3& color);
}
