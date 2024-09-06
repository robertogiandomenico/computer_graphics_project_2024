// This is the main: probably you do not need to touch this!
#include <iostream>
#include <exception>
#include <memory>

#include "PurrfectPotion.hpp"

int main() {
	std::unique_ptr<PurrfectPotion> app = std::make_unique<PurrfectPotion>();

	try {
		app->run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}