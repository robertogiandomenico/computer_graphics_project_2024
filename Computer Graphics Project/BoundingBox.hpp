#include <glm/glm.hpp>
#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time() - to seed rand()

#include "Utils.hpp"

#define COLLECTIBLES_NUM 7

struct BoundingBox {
	glm::vec3 min;
	glm::vec3 max;
	std::string name;

	// Constructor
	BoundingBox(const std::string& name, const glm::vec3 center, const glm::vec3 size) {
		this->name = name;
		glm::vec3 halfSize = size / 2.0f;

		glm::vec3 shiftedCenter = center; // + glm::vec3(0, halfSize.y, 0);

		min = shiftedCenter - halfSize;
		max = shiftedCenter + halfSize;
	}

	// Check if this bounding box intersects with another
	bool intersects(BoundingBox& other) {
		return (min.x <= other.max.x && max.x >= other.min.x) &&
			(min.y <= other.max.y && max.y >= other.min.y) &&
			(min.z <= other.max.z && max.z >= other.min.z);
	}

	// Update the position of the bounding box
	void updatePosition(const glm::vec3& center) {
		glm::vec3 halfSize = (max - min) / 2.0f;
		min = center - halfSize;
		max = center + halfSize;
	}

	// Update the size of the bounding box
	void erase() {
		min = glm::vec3(0);
		max = glm::vec3(0);
	}

	std::string getName() {
		return name;
	}
};

struct VertexBoundingBox {
	glm::vec3 pos;
};

struct BoundingBoxUniformBlock {
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

void createBBModel(std::vector<VertexBoundingBox>& vDef, std::vector<uint32_t>& vIdx, BoundingBox* bb) {
    glm::vec3 halfExtents = (bb->max - bb->min) / 2.0f;

    // 8 vertices of the bounding box
    vDef.push_back({ glm::vec3(halfExtents.x, halfExtents.y, halfExtents.z) });   // 0
    vDef.push_back({ glm::vec3(halfExtents.x, halfExtents.y, -halfExtents.z) });  // 1
    vDef.push_back({ glm::vec3(-halfExtents.x, halfExtents.y, halfExtents.z) });  // 2
    vDef.push_back({ glm::vec3(-halfExtents.x, halfExtents.y, -halfExtents.z) }); // 3
    vDef.push_back({ glm::vec3(halfExtents.x, -halfExtents.y, halfExtents.z) });  // 4
    vDef.push_back({ glm::vec3(halfExtents.x, -halfExtents.y, -halfExtents.z) }); // 5
    vDef.push_back({ glm::vec3(-halfExtents.x, -halfExtents.y, halfExtents.z) }); // 6
    vDef.push_back({ glm::vec3(-halfExtents.x, -halfExtents.y, -halfExtents.z) }); // 7

    // indices for the 12 triangles that make up the 6 faces of the box
    // TOP
    vIdx.push_back(0); vIdx.push_back(1); vIdx.push_back(2);
    vIdx.push_back(1); vIdx.push_back(3); vIdx.push_back(2);

    // BOTTOM
    vIdx.push_back(4); vIdx.push_back(6); vIdx.push_back(5);
    vIdx.push_back(5); vIdx.push_back(6); vIdx.push_back(7);

    // FRONT
    vIdx.push_back(0); vIdx.push_back(2); vIdx.push_back(4);
    vIdx.push_back(4); vIdx.push_back(2); vIdx.push_back(6);

    // BACK
    vIdx.push_back(1); vIdx.push_back(5); vIdx.push_back(3);
    vIdx.push_back(5); vIdx.push_back(7); vIdx.push_back(3);

    // LEFT
    vIdx.push_back(2); vIdx.push_back(3); vIdx.push_back(6);
    vIdx.push_back(3); vIdx.push_back(7); vIdx.push_back(6);

    // RIGHT
    vIdx.push_back(0); vIdx.push_back(4); vIdx.push_back(1);
    vIdx.push_back(1); vIdx.push_back(4); vIdx.push_back(5);
}

void drawBoundingBox(bool hasBoundingBox, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::mat4 ViewPrj,
					BoundingBoxUniformBlock UBO_boundingBox, DescriptorSet DS_boundingBox, int currentImage) {
	glm::mat4 World;
		
	if (hasBoundingBox) {	// set hasBoundingBox to false to not display the bounding box
		World = glm::translate(glm::mat4(1), position) *
			glm::rotate(glm::mat4(1), rotation.x, glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1), rotation.z, glm::vec3(0, 0, 1)) *
			glm::scale(glm::mat4(1), scale);
	}
	else {
		World = glm::scale(glm::mat4(1), glm::vec3(0.0f));	// scale to zero to not display the bounding box
	}
	UBO_boundingBox.mvpMat = ViewPrj * World;
	UBO_boundingBox.mvpMat = ViewPrj * World;
	UBO_boundingBox.mMat = World;
	UBO_boundingBox.nMat = glm::inverse(glm::transpose(World));
	DS_boundingBox.map(currentImage, &UBO_boundingBox, sizeof(UBO_boundingBox), 0);
}

void emptyBBList(std::vector<BoundingBox>* BBList) {
	BBList->clear();
}

void fillBBList(std::vector<BoundingBox>* BBList, glm::vec3* BBPosition) {

	srand(static_cast<unsigned int>(time(0)));
	for (int i = 0; i < COLLECTIBLES_NUM; ++i) {
		glm::vec3 randomPosition = generateRandomPosition(10.3f);

		// make sure collectibles are not spawned in the same position or too close to each other
		for (int j = 0; j < i; ++j) {
			if (glm::distance(randomPosition, BBPosition[j]) < 2.0f) {
				randomPosition = generateRandomPosition(10.3f);
				j = -1;
			}
		}
		BBPosition[i] = randomPosition;
	}

	// Create the bounding boxes
	BBList->push_back(BoundingBox("crystal", BBPosition[0], glm::vec3(0.7f)));
	BBList->push_back(BoundingBox("eye",	 BBPosition[1], glm::vec3(0.5f)));
	BBList->push_back(BoundingBox("feather", BBPosition[2], glm::vec3(0.5f, 0.6f, 0.9f)));
	BBList->push_back(BoundingBox("leaf",	 BBPosition[3], glm::vec3(0.6f, 0.5f, 0.5f)));
	BBList->push_back(BoundingBox("potion1", BBPosition[4], glm::vec3(0.5f, 1.0f, 0.5f)));
	BBList->push_back(BoundingBox("potion2", BBPosition[5], glm::vec3(0.5f, 1.0f, 0.5f)));
	BBList->push_back(BoundingBox("bone",	 BBPosition[6], glm::vec3(0.5f, 0.7f, 0.5f)));
}