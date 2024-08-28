#include <glm/glm.hpp>
#include <string>

#define COLLECTIBLES_NUM 7

std::string collectiblesNames[COLLECTIBLES_NUM] = {
	"crystal",
	"eye",
	"feather",
	"leaf",
	"potion1",
	"potion2",
	"bone"
};


glm::vec3 generateRandomPosition(int squareSide) {
	glm::vec3 randomPosition;

	randomPosition = glm::vec3(
		-squareSide + rand() % (squareSide * 2 + 1),
		0.4f,
		-squareSide + rand() % (squareSide * 2 + 1)
	);

	return randomPosition;
}