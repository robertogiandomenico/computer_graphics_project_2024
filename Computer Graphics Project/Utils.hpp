#include <glm/glm.hpp>

glm::vec3 generateRandomPosition(int lowerBound, int upperBound) {
	glm::vec3 randomPosition;

	randomPosition = glm::vec3(
		lowerBound + rand() % (upperBound - lowerBound + 1),
		0.3f,
		lowerBound + rand() % (upperBound - lowerBound + 1)
	);

	return randomPosition;
}