# include <glm/glm.hpp>

struct BoundingBox {
	glm::vec3 min;
	glm::vec3 max;

	BoundingBox(const glm::vec3 center, const glm::vec3 size) {
		glm::vec3 halfSize = size / 2.0f;
		min = center - halfSize;
		max = center + halfSize;
	}

	bool intersects(BoundingBox& other) {
		return (min.x <= other.max.x && max.x >= other.min.x) &&
			(min.y <= other.max.y && max.y >= other.min.y) &&
			(min.z <= other.max.z && max.z >= other.min.z);
	}
};