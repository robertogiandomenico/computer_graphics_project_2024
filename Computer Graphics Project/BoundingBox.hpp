# include <glm/glm.hpp>

struct BoundingBox {
	glm::vec3 min;
	glm::vec3 max;
	std::string name;

	// Constructor
	BoundingBox(const std::string& name, const glm::vec3 center, const glm::vec3 size) {
		this->name = name;
		glm::vec3 halfSize = size / 2.0f;
		min = center - halfSize;
		max = center + halfSize;
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