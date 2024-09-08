#pragma once

#include <vector>
#include <map>
#include <string>

#include "Starter.hpp"
#include "BoundingBox.hpp"
#include "Utils.hpp"
#include "World.hpp"

class PurrfectPotion : public BaseProject {
protected:
	// Current aspect ratio (used by the callback that resizes the window)
	float Ar;

	// Other application parameters
	// Camera position, orientation, distance from target
	glm::vec3 camPos;
	float camYaw;
	float camPitch;
	float camRoll;
	float camDist;
	glm::vec3 CamTargetDelta = glm::vec3(0.0f);
	float minPitch = glm::radians(-20.0f);
	float maxPitch = M_PI_2 - 0.1f;
	float minRoll = -M_PI_2;
	float maxRoll = M_PI_2;

	// Cat position and orientation
	glm::vec3 catPosition;
	glm::vec3 catDimensions = glm::vec3(1.2f, 1.2f, 0.3f);
	float catYaw;

	// Timer setup
	const float GAME_DURATION = 180.0f;		// 3 minutes = 180 seconds
	float totalElapsedTime = 0.0f;			// in seconds
	float timeLeft = GAME_DURATION;
	int lastDisplayedTime = static_cast<int>(GAME_DURATION);

	// Game state variables
	bool DEBUG = false;							// to display bounding boxes for debugging
	bool OVERLAY = false;						// to display the overlay
	bool FIRST_PERSON = false;					// to switch between first and third person view
	bool gameOver = false;						// to determine when all the collectibles have been collected
	bool cursorShowed = false;					// to show/hide the cursor
	int gameState = GAME_STATE_START_SCREEN;	// initially state of the game = start screen
	glm::vec4 lightOn = glm::vec4(1, 1, 0, 1);	// initially all types of light are on, except spot

	// Collectibles parameters
	float collectibleRotationAngle = 0.0f;						 // rotation angle
	const float collectibleRotationSpeed = glm::radians(45.0f);  // rotation speed: 45 degrees per second
	glm::vec3 collectiblesRandomPosition[COLLECTIBLES_NUM];		 // position

	std::map<std::string, bool> collectiblesMap;
	std::vector<BoundingBox> collectiblesBBs;
	std::map<std::string, int> collectiblesHUD = {
		{"crystal", 0},
		{"eye",		1},
		{"feather", 2},
		{"leaf",	3},
		{"potion1", 4},
		{"potion2", 5},
		{"bone",	6}
	};

	float ROT_SPEED = glm::radians(150.0f);
	float MOVE_SPEED = 6.0f;
	glm::vec3 cameraForward;
	glm::vec3 cameraRight;

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSL, DSL_skyBox, DSL_animated, DSL_overlay, DSL_ward, DSL_boundingBox, DSL_DRN, DSL_global;

	// Vertex formats
	VertexDescriptor VD, VD_skyBox, VD_overlay, VD_tangent, VD_boundingBox;

	// Pipelines [Shader couples]
	Pipeline P, P_skyBox, P_animated, P_overlay, P_ward, P_boundingBox, P_DRN, P_cat;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure

	// Models
	Model<Vertex>   // Bathroom
		M_bathtub, M_bidet, M_sink, M_toilet,
		// Bedroom
		M_bed, M_closet, M_nighttable,
		// Collectibles
		M_bone, M_crystal, M_eye, M_feather, M_leaf, M_potion1, M_potion2,
		// Kitchen		  
		M_chair, M_fridge, M_kitchen, M_kitchentable,
		// Lair
		M_cauldron, M_stonechair, M_chest, M_shelf1, M_shelf2, M_stonetable, M_steam, M_fire, M_web,
		// Living room
		M_sofa, M_table, M_tv,
		// Other
		M_cat;

	Model<VertexTan> M_knight, M_floor, M_walls, M_catFainted;
	Model<skyBoxVertex> M_skyBox;
	Model<VertexOverlay> M_timer[5], M_screens[4], M_scroll, M_collectibles[COLLECTIBLES_NUM];
	std::vector<Model<VertexBoundingBox>> M_boundingBox;

	// Descriptor sets
	DescriptorSet   // Bathroom
		DS_bathtub, DS_bidet, DS_sink, DS_toilet,
		// Bedroom
		DS_bed, DS_closet, DS_nighttable,
		// Collectibles
		DS_bone, DS_crystal, DS_eye, DS_feather, DS_leaf, DS_potion1, DS_potion2,
		// Kitchen
		DS_chair, DS_fridge, DS_kitchen, DS_kitchentable,
		// Lair
		DS_cauldron, DS_stonechair, DS_chest, DS_shelf1, DS_shelf2, DS_stonetable, DS_steam, DS_fire, DS_web, DS_catFainted,
		// Living room
		DS_sofa, DS_table, DS_tv, DS_knight,
		// Other
		DS_cat, DS_floor, DS_walls, DS_global, DS_skyBox,
		// HUD
		DS_timer[5], DS_screens[4], DS_scroll, DS_collectibles[COLLECTIBLES_NUM];

	std::vector<DescriptorSet> DS_boundingBox;

	// Textures
	Texture T_textures, T_eye, T_closet, T_feather, T_knight[3], T_skyBox, T_steam, T_fire, T_timer[5], T_screens[4], T_scroll, T_collectibles[COLLECTIBLES_NUM],
		T_catDiffuseGhost, T_cat[3], T_wall[3], T_floor[3];

	// C++ storage for uniform variables
	UniformBufferObject // Bathroom
		UBO_bathtub, UBO_bidet, UBO_sink, UBO_toilet,
		// Bedroom
		UBO_bed, UBO_closet, UBO_nightTable,
		// Collectibles
		UBO_bone, UBO_crystal, UBO_eye, UBO_feather, UBO_leaf, UBO_potion1, UBO_potion2,
		// Kitchen
		UBO_chair, UBO_fridge, UBO_kitchen, UBO_kitchenTable,
		// Lair
		UBO_cauldron, UBO_stoneChair, UBO_chest, UBO_shelf1, UBO_shelf2, UBO_stoneTable, UBO_web, UBO_catFainted,
		// Living room
		UBO_sofa, UBO_table, UBO_tv, UBO_knight,
		// Other
		UBO_floor, UBO_walls;

	std::vector<UniformBufferObject> UBO_boundingBox;
	AnimatedUniformBufferObject UBO_steam, UBO_fire, UBO_cat;
	SkyBoxUniformBufferObject UBO_skyBox;
	OverlayUniformBlock UBO_timer[5], UBO_screens[4], UBO_scroll, UBO_collectibles[COLLECTIBLES_NUM];
	GlobalUniformBufferObject GUBO;

	// To display the bounding boxes for debugging
	std::vector<BoundingBox> furnitureBBs;
	BoundingBox catBox = BoundingBox("cat", catPosition, catDimensions);

	// Here you set the main application parameters
	void setWindowParameters();

	// What to do when the window changes size
	void onWindowResize(int w, int h);

	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit();

	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit();

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup();

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup();

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw, with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage);

	// Here is where you update the uniforms. Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage);

	// Check for collisions with collectibles and furniture
	void checkCollisions(uint32_t currentImage, glm::vec3& m, float deltaT);

	// Position all the objects in the world
	void worldSetUp(const glm::vec3& catNewPos, const glm::mat4& ViewPrj, uint32_t currentImage);

	// Position ghost cat, draw its bounding box (if DEBUG) and update its uniform
	void placeGhostCat(AnimatedUniformBufferObject ubo, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 emissiveColor,
									glm::mat4 ViewPrj, DescriptorSet ds, int currentImage, bool hasBoundingBox, int id);

	// Position objects (furniture, collectibles, fainted cat), draw their bounding box (if DEBUG) and update their uniforms
	void placeEntity(UniformBufferObject ubo, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale,
		glm::vec3 emissiveColor, glm::mat4 ViewPrj, DescriptorSet ds, int currentImage, bool hasBoundingBox, int id = 0);

	// Remove a collectible from the scene
	void removeCollectible(UniformBufferObject ubo, glm::mat4 ViewPrj, DescriptorSet ds, int currentImage, int id);

	// Update overlay elements
	void updateOverlay(uint32_t currentImage);

	// Update steam and fire UBOs
	void updateSteamAndFire(glm::mat4& World, glm::mat4& ViewPrj, uint32_t currentImage);

	// Setup all the lights in the scene
	void updateLights(uint32_t currentImage);

	// Create menu scenes placing the cat and the camera in a fixed position
	void updateMenuScene(bool start, uint32_t currentImage);

	// Update all the game elements (cat and camera position, time), also based on buttons pressed
	void updateGame(bool& debounce, int& curDebounce, float& deltaT, glm::vec3& m, glm::vec3& r, uint32_t currentImage);

	// Check whether a button is pressed and update the game elements accordingly
	void checkPressedButton(bool* debounce, int* curDebounce);

	// Show the cursor, unlocking it from the window
	void showCursor();

	// Hide the cursor, locking it to the window
	void hideCursor();
};
