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

	std::string getName() {
		return name;
	}
};