#ifndef WORLD_HPP
#define WORLD_HPP

#include <glm/glm.hpp>

struct Transform {
    glm::vec3 pos;
    glm::vec3 rot;
    glm::vec3 scale;
};

// Define as an inline variable to make only one instance of it

//House
inline Transform houseFloor = {
    glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform walls = {
	glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f)
};

// Bedroom
inline Transform closet = {
     glm::vec3(6.f, 0.0f, -12.f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform bed = {
     glm::vec3(11.f, 0.0f, -10.f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform nightTable = {
     glm::vec3(11.9f, 0.0f, -4.f), glm::vec3(0.0f), glm::vec3(1.0f)
};

// Kitchen
inline Transform kitchen = {
     glm::vec3(9.5f, 0.0f, 11.6f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform fridge = {
     glm::vec3(0.f, 0.0f, 11.8f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform kitchenTable = {
     glm::vec3(7.f, 0.0f, 7.7f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform chair = {
     glm::vec3(7.f, 0.0f, 6.8f), glm::vec3(0.0f), glm::vec3(1.0f)
};

// Living Room
inline Transform sofa = {
     glm::vec3(-11.5f, 0.0f, 9.5f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform table = {
     glm::vec3(-7.f, 0.0f, 5.f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform tv = {
     glm::vec3(-7.5f, 0.0f, 9.5f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform knight = {
     glm::vec3(-2.f, 0.0f, 6.5f), glm::vec3(0.0f), glm::vec3(1.0f)
};

// Witch lair
inline Transform chest = {
     glm::vec3(-7.f, 0.0f, -12.0f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform stoneTable = {
     glm::vec3(-11.6f, 0.0f, -10.f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform stoneChair = {
     glm::vec3(-10.8f, 0.0f, -10.f), glm::vec3(0.f, glm::radians(25.f), 0.f), glm::vec3(1.0f)
};
inline Transform cauldron = {
     glm::vec3(-6.0f, 0.0f, -8.3f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform shelf1 = {
     glm::vec3(-12.2f, 2.f, -5.5f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform shelf2 = {
     glm::vec3(-9.f, 2.4f, -12.2f), glm::vec3(0, glm::radians(-90.f), 0), glm::vec3(1.0f)
};
inline Transform web = {
     glm::vec3(-12.f, 2.5f, -12.f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform catFainted = {
     glm::vec3(-8.5f, 0.f, -10.0f), glm::vec3(0.0f), glm::vec3(1.0f)
};

// Bathroom
inline Transform bathtub = {
     glm::vec3(-1.4f, 0.0f, -11.8f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform toilet = {
     glm::vec3(1.3f, 0.0f, -8.5f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform bidet = {
     glm::vec3(1.3f, 0.0f, -6.5f), glm::vec3(0.0f), glm::vec3(1.0f)
};
inline Transform sink = {
     glm::vec3(-3.4f, 0.0f, -7.f), glm::vec3(0.0f), glm::vec3(1.0f)
};


#endif