#include <glm/glm.hpp>

glm::vec3 generateRandomPosition(int squareSide) {
	glm::vec3 randomPosition;

	randomPosition = glm::vec3(
		-squareSide + rand() % (squareSide * 2 + 1),
		0.3f,
		-squareSide + rand() % (squareSide * 2 + 1)
	);

	return randomPosition;
}