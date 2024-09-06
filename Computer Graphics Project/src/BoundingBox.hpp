#pragma once

#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time() - to seed rand()
#include <vector>

#include "Starter.hpp"
#include "Utils.hpp"

struct BoundingBox {
	glm::vec3 min;
	glm::vec3 max;
	std::string name;

	// Constructor
	BoundingBox(const std::string& name, const glm::vec3 center, const glm::vec3 size);

	// Check if this bounding box intersects with another
	bool intersects(BoundingBox& other);

	// Update the size of the bounding box
	void erase();

	std::string getName();
};

struct VertexBoundingBox {
	glm::vec3 pos;
};

void createBBModel(std::vector<VertexBoundingBox>& vDef, std::vector<uint32_t>& vIdx, BoundingBox* bb);

void drawBoundingBox(bool hasBoundingBox, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::mat4 ViewPrj,
	UniformBufferObject UBO_boundingBox, DescriptorSet DS_boundingBox, int currentImage);

void emptyBBList(std::vector<BoundingBox>* BBList);

void fillBBList(std::vector<BoundingBox>* BBList, glm::vec3* BBPosition);
