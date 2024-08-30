#ifndef UTILS_HPP
#define UTILS_HPP

#include <glm/glm.hpp>
#include <string>

#define COLLECTIBLES_NUM 7
#define GAME_STATE_START_SCREEN 0
#define GAME_STATE_PLAY 1
#define GAME_STATE_GAME_WIN 2
#define GAME_STATE_GAME_LOSE 3

#define PI 3.14159265359

glm::vec3 generateRandomPosition(int squareSide) {
	glm::vec3 randomPosition;

	randomPosition = glm::vec3(
		-squareSide + rand() % (squareSide * 2 + 1),
		0.4f,
		-squareSide + rand() % (squareSide * 2 + 1)
	);

	return randomPosition;
}

#endif