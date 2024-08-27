// This has been adapted from the Vulkan tutorial
#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <map>
#include <string>
#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time() - to seed rand()

#include "Starter.hpp"
#include "BoundingBox.hpp"
#include "Utils.hpp"
#include "World.hpp"

#define LIGHTS_NUM 7
#define COLLECTIBLES_NUM 7

// The uniform buffer objects data structures
// Remember to use the correct alignas(...) value
//        float : alignas(4)
//        vec2  : alignas(8)
//        vec3  : alignas(16)
//        vec4  : alignas(16)
//        mat3  : alignas(16)
//        mat4  : alignas(16)
// Example:
struct UniformBufferObject {
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

struct GlobalUniformBufferObject {
	alignas(16) glm::vec3 lightPos[LIGHTS_NUM];     // Position of the lights
	alignas(16) glm::vec3 lightColor[LIGHTS_NUM];   // Color of the lights
	alignas(16) glm::vec3 eyePos;					// Position of the camera/eye
	alignas(4) float constant[LIGHTS_NUM];			// Constant attenuation factor
	alignas(4) float linear[LIGHTS_NUM];			// Linear attenuation factor
	alignas(4) float quadratic[LIGHTS_NUM];			// Quadratic attenuation factor
};

struct SkyBoxUniformBufferObject {
	alignas(16) glm::mat4 mvpMat;				// Field for MVP matrix
	alignas(4) float time;						// Field for time
};

struct SteamUniformBufferObject {
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
	alignas(4) float time;						// Time variable for animation
};

struct OverlayUniformBlock {
	alignas(4) float visible;
};

// The vertices data structures
// Example
struct Vertex {
	glm::vec3 pos;
	glm::vec2 UV;
	glm::vec3 norm;
};

struct skyBoxVertex {
	glm::vec3 pos;
};

struct VertexOverlay {
	glm::vec2 pos;
	glm::vec2 UV;
};


// MAIN ! 
class PurrfectPotion : public BaseProject {
protected:

	// Current aspect ratio (used by the callback that resized the window)
	float Ar;

	// Other application parameters
	glm::vec3 camPos = glm::vec3(0.0, 1.5, 7.0);
	float camYaw = glm::radians(90.0f);
	float camPitch = glm::radians(-10.0f);
	float camRoll = 0.0f;
	float camDist = 3.0f;
	const glm::vec3 CamTargetDelta = glm::vec3(0, 1.5f, 0);
	//const glm::vec3 Cam1stPos = glm::vec3(0.49061f, 2.07f, 2.7445f);
	float Yaw = 0.0f;
	// Rotation angle for the cube
	float collectibleRotationAngle = 0.0f;
	// Rotation speed in radians per second
	const float collectibleRotationSpeed = glm::radians(45.0f);  // 45 degrees per second

	// Cat initial position
	glm::vec3 catPosition = glm::vec3(6.0f, 0.0f, 0.0f);
	glm::vec3 catDimensions = glm::vec3(1.2f, 1.2f, 0.25f);
	// Cat initial orientation
	float catYaw = 0.f;

	glm::vec3 collectiblesRandomPosition[COLLECTIBLES_NUM];

	// Timer setup
	const float GAME_DURATION = 120.0f;		// 2 minutes = 120 seconds
	float totalElapsedTime = 0.0f;			// in seconds
	float remainingTime = GAME_DURATION;	
	int lastDisplayedTime = static_cast<int>(GAME_DURATION);

	float minimumPressDelay = 0.1f;
	float lastPressTime = 0.0f;

	bool DEBUG = false;						// Used to display bounding boxes for debugging
	bool OVERLAY = true;					// Used to display the overlay

	public:
		std::map<std::string, bool> collectiblesMap;
		std::map<std::string, int> collectiblesHUD;

		PurrfectPotion() {

			// Initialize items with names and set all isCollected to false
			collectiblesMap["crystal"] = false;
			collectiblesMap["eye"] = false;
			collectiblesMap["feather"] = false;
			collectiblesMap["leaf"] = false;
			collectiblesMap["potion1"] = false;
			collectiblesMap["potion2"] = false;
			collectiblesMap["bone"] = false;

			collectiblesHUD["crystal"] = 0;
			collectiblesHUD["eye"] = 1;
			collectiblesHUD["feather"] = 2;
			collectiblesHUD["leaf"] = 3;
			collectiblesHUD["potion1"] = 4;
			collectiblesHUD["potion2"] = 5;
			collectiblesHUD["bone"] = 6;

			/*for (int i = 0; i < COLLECTIBLES_NUM; i++) {
				UBO_collectibles[i].visible = 1.f;
			}*/

			srand(static_cast<unsigned int>(time(0)));
			for (int i = 0; i < COLLECTIBLES_NUM; ++i) {
				glm::vec3 randomPosition = generateRandomPosition(10.3f);

				// make sure collectibles are not spawned in the same position or too close to each other
				for (int j = 0; j < i; ++j) {
					if (glm::distance(randomPosition, collectiblesRandomPosition[j]) < 2.0f) {
						randomPosition = generateRandomPosition(10.3f);
						j = -1;
					}
				}
				collectiblesRandomPosition[i] = randomPosition;
			}
		}

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSL, DSL_skyBox, DSL_steam, DSL_overlay;

	// Vertex formats
	VertexDescriptor VD, VD_skyBox, VD_overlay;

	// Pipelines [Shader couples]
	Pipeline P, P_skyBox, P_steam, P_overlay;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	// Models
	// Bathroom
	Model<Vertex> M_bathtub, M_bidet, M_sink, M_toilet;
	// Bedroom
	Model<Vertex> M_bed, M_closet, M_nighttable;
	// Collectibles
	Model<Vertex> M_bone, M_crystal, M_eye, M_feather, M_leaf, M_potion1, M_potion2;
	// Kitchen
	Model<Vertex> M_chair, M_fridge, M_kitchen, M_kitchentable;
	// Lair
	Model<Vertex> M_cauldron, M_stonechair, M_chest, M_shelf1, M_shelf2, M_stonetable;
	// Living room
	Model<Vertex> M_sofa, M_table, M_tv;
	// Other
	Model<Vertex> M_cat, M_floor, M_walls, M_steam;
	Model<skyBoxVertex> M_skyBox;
	Model<VertexOverlay> M_timer[5], M_scroll, M_collectibles[COLLECTIBLES_NUM];

	// Descriptor sets
	// Bathroom
	DescriptorSet DS_bathtub, DS_bidet, DS_sink, DS_toilet;
	// Bedroom
	DescriptorSet DS_bed, DS_closet, DS_nighttable;
	// Collectibles
	DescriptorSet DS_bone, DS_crystal, DS_eye, DS_feather, DS_leaf, DS_potion1, DS_potion2;
	// Kitchen
	DescriptorSet DS_chair, DS_fridge, DS_kitchen, DS_kitchentable;
	// Lair
	DescriptorSet DS_cauldron, DS_stonechair, DS_chest, DS_shelf1, DS_shelf2, DS_stonetable;
	// Living room
	DescriptorSet DS_sofa, DS_table, DS_tv;
	// Other
	DescriptorSet DS_cat, DS_floor, DS_walls, DS_steam;

	DescriptorSet DS_skyBox, DS_timer[5], DS_scroll, DS_collectibles[COLLECTIBLES_NUM];

	// Textures
	Texture T_textures, T_eye, T_closet, T_feather, T_skyBox, T_steam, T_timer[5], T_scroll, T_collectibles[COLLECTIBLES_NUM];

	// C++ storage for uniform variables
	// Bathroom
	UniformBufferObject UBO_bathtub, UBO_bidet, UBO_sink, UBO_toilet;
	// Bedroom
	UniformBufferObject UBO_bed, UBO_closet, UBO_nightTable;
	// Collectibles
	UniformBufferObject UBO_bone, UBO_crystal, UBO_eye, UBO_feather, UBO_leaf, UBO_potion1, UBO_potion2;
	// Kitchen
	UniformBufferObject UBO_chair, UBO_fridge, UBO_kitchen, UBO_kitchenTable;
	// Lair
	UniformBufferObject UBO_cauldron, UBO_stoneChair, UBO_chest, UBO_shelf1, UBO_shelf2, UBO_stoneTable;
	// Living room
	UniformBufferObject UBO_sofa, UBO_table, UBO_tv;
	// Other
	UniformBufferObject UBO_cat, UBO_floor, UBO_walls;
	SteamUniformBufferObject UBO_steam;
	OverlayUniformBlock UBO_timer[5], UBO_scroll, UBO_collectibles[COLLECTIBLES_NUM];

	// to display the bounding boxes for debugging
	Pipeline P_boundingBox;
	DescriptorSetLayout DSL_boundingBox;
	VertexDescriptor VD_boundingBox;
	std::vector<Model<VertexBoundingBox>> M_boundingBox;
	std::vector<DescriptorSet> DS_boundingBox;
	std::vector<BoundingBoxUniformBlock> UBO_boundingBox;

	std::vector<BoundingBox> collectiblesBBs;
	std::vector<BoundingBox> fornitureBBs;
	BoundingBox catBox = BoundingBox("cat", catPosition, catDimensions);

	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, title and initial background
		windowWidth = 1200;
		windowHeight = 800;
		windowTitle = "Purrfect Potion";
		windowResizable = GLFW_TRUE;
		initialBackgroundColor = { 0.5f, 0.5f, 0.5f, 1.0f };

		// Descriptor pool sizes
		uniformBlocksInPool = 200; //30 but works with >= 93
		texturesInPool = 100;	  //5 but works with >= 29
		setsInPool = 200;		  //31

		Ar = (float)windowWidth / (float)windowHeight;
	}

	// What to do when the window changes size
	// for now not used because windowResizable is set to GLFW_FALSE
	void onWindowResize(int w, int h) {
		std::cout << "Window resized to: " << w << " x " << h << "\n";
		Ar = (float)w / (float)h;
	}

	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() {

		// Create the bounding boxes
		collectiblesBBs.push_back(BoundingBox("crystal", collectiblesRandomPosition[0], glm::vec3(0.7f)));
		collectiblesBBs.push_back(BoundingBox("eye", collectiblesRandomPosition[1], glm::vec3(0.5f)));
		collectiblesBBs.push_back(BoundingBox("feather", collectiblesRandomPosition[2], glm::vec3(0.5f, 0.6f, 0.9f)));
		collectiblesBBs.push_back(BoundingBox("leaf", collectiblesRandomPosition[3], glm::vec3(0.6f, 0.5f, 0.5f)));
		collectiblesBBs.push_back(BoundingBox("potion1", collectiblesRandomPosition[4], glm::vec3(0.5f, 1.0f, 0.5f)));
		collectiblesBBs.push_back(BoundingBox("potion2", collectiblesRandomPosition[5], glm::vec3(0.5f, 1.0f, 0.5f)));
		collectiblesBBs.push_back(BoundingBox("bone", collectiblesRandomPosition[6], glm::vec3(0.5f, 0.7f, 0.5f)));

		fornitureBBs.push_back(BoundingBox("bathtub", bathtub.pos, glm::vec3(3.3f, 1.4f, 1.4f)));
		fornitureBBs.push_back(BoundingBox("closet", closet.pos, glm::vec3(5.6f, 3.2f, 1.f)));
		fornitureBBs.push_back(BoundingBox("bed", bed.pos, glm::vec3(2.f, 1.2f, 4.5f)));
		fornitureBBs.push_back(BoundingBox("nightTable", nightTable.pos, glm::vec3(0.88, 1.2f, 1.1f)));
		fornitureBBs.push_back(BoundingBox("chest", chest.pos, glm::vec3(1.4f, 1.5f, 0.7f)));
		fornitureBBs.push_back(BoundingBox("sofa", sofa.pos, glm::vec3(1.f, 1.1f, 3.f)));
		fornitureBBs.push_back(BoundingBox("fridge", fridge.pos, glm::vec3(1.5f, 2.7f, 1.5f)));
		fornitureBBs.push_back(BoundingBox("kitchen", kitchen.pos, glm::vec3(5.f, 3.3f, 1.72f)));

		// create ubo needed for the bounding boxes (debug)
		for (int i = 0; i < collectiblesBBs.size() + fornitureBBs.size() + 1; i++) {
			UBO_boundingBox.push_back(BoundingBoxUniformBlock());
		}

		// Descriptor Layouts [what will be passed to the shaders]
		DSL.init(this, {
			// this array contains the bindings:
			// first  element : the binding number
			// second element : the type of element (buffer or texture)
			//                  using the corresponding Vulkan constant
			// third  element : the pipeline stage where it will be used
			//                  using the corresponding Vulkan constant
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
			{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
			{3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT}			// New binding for emissive color
		});

		DSL_skyBox.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},			// Uniform buffer for MVP matrix
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},	// Combined image sampler for skybox texture
			{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT}			// Uniform buffer for time
		});

		DSL_steam.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},				// Steam UBO binding
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}	// Steam texture binding
		});

		DSL_boundingBox.init(this, {
				{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
		});

		DSL_overlay.init(this, {
				{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
				{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
		});

		// Vertex descriptors
		VD.init(this, {
			// this array contains the bindings
			// first  element : the binding number
			// second element : the stride of this binging
			// third  element : whether this parameter change per vertex or per instance
			//                  using the corresponding Vulkan constant
			{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				// this array contains the location
				// first  element : the binding number
				// second element : the location number
				// third  element : the offset of this element in the memory record
				// fourth element : the data type of the element
				//                  using the corresponding Vulkan constant
				// fifth  elmenet : the size in byte of the element
				// sixth  element : a constant defining the element usage
				//                   POSITION - a vec3 with the position
				//                   NORMAL   - a vec3 with the normal vector
				//                   UV       - a vec2 with a UV coordinate
				//                   COLOR    - a vec4 with a RGBA color
				//                   TANGENT  - a vec4 with the tangent vector
				//                   OTHER    - anything else
				//
				// ***************** DOUBLE CHECK ********************
				//    That the Vertex data structure you use in the "offsetoff" and
				//	in the "sizeof" in the previous array, refers to the correct one,
				//	if you have more than one vertex format!
				// ***************************************************
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos),
					   sizeof(glm::vec3), POSITION},
				{0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, UV),
					   sizeof(glm::vec2), UV},
				{0, 2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, norm),
					   sizeof(glm::vec3), NORMAL}
		});

		VD_skyBox.init(this, {
			  {0, sizeof(skyBoxVertex), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
			  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(skyBoxVertex, pos),
					sizeof(glm::vec3), POSITION}
		});

		VD_boundingBox.init(this, {
			{0, sizeof(VertexBoundingBox), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexBoundingBox, pos),
					   sizeof(glm::vec3), POSITION}
		});

		VD_overlay.init(this, {
			{0, sizeof(VertexOverlay), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
				  {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, pos),
						 sizeof(glm::vec2), OTHER},
				  {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, UV),
						 sizeof(glm::vec2), UV}
			});
			
			// Pipelines [Shader couples]
			// The second parameter is the pointer to the vertex definition
			// Third and fourth parameters are respectively the vertex and fragment shaders
			// The last array, is a vector of pointer to the layouts of the sets that will
			// be used in this pipeline. The first element will be set 0, and so on..
			P.init(this, &VD, "shaders/ShaderVert.spv", "shaders/ShaderFrag.spv", { &DSL });
			P.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);

			P_skyBox.init(this, &VD_skyBox, "shaders/SkyBoxVert.spv", "shaders/SkyBoxFrag.spv", { &DSL_skyBox });
			P_skyBox.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, false);

			P_steam.init(this, &VD, "shaders/SteamVert.spv", "shaders/SteamFrag.spv", { &DSL_steam });
			P_steam.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, true);

			P_boundingBox.init(this, &VD_boundingBox, "shaders/BoundingBoxVert.spv", "shaders/BoundingBoxFrag.spv", { &DSL_boundingBox });
			P_boundingBox.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_LINE, VK_CULL_MODE_BACK_BIT, false);

			P_overlay.init(this, &VD_overlay, "shaders/OverlayVert.spv", "shaders/OverlayFrag.spv", { &DSL_overlay });
			P_overlay.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, true);

			// Models, textures and Descriptors (values assigned to the uniforms)

			// Create models
			// The second parameter is the pointer to the vertex definition for this model
			// The third parameter is the file name
			// The last is a constant specifying the file type: currently only OBJ or GLTF
			M_bathtub.init(this, &VD, "models/bathroom/bathroom_bathtub.gltf", GLTF);
			M_bidet.init(this, &VD, "models/bathroom/bathroom_bidet.gltf", GLTF);
			M_sink.init(this, &VD, "models/bathroom/bathroom_sink.gltf", GLTF);
			M_toilet.init(this, &VD, "models/bathroom/bathroom_toilet.gltf", GLTF);

			M_bed.init(this, &VD, "models/bedroom/bedroom_bed.gltf", GLTF);
			M_closet.init(this, &VD, "models/bedroom/bedroom_closet.gltf", GLTF);
			M_nighttable.init(this, &VD, "models/bedroom/bedroom_night_table.gltf", GLTF);

			M_bone.init(this, &VD, "models/collectibles/coll_bone.gltf", GLTF);
			M_crystal.init(this, &VD, "models/collectibles/coll_crystal.gltf", GLTF);
			M_eye.init(this, &VD, "models/collectibles/coll_eye.gltf", GLTF);
			M_feather.init(this, &VD, "models/collectibles/coll_feather.gltf", GLTF);
			M_leaf.init(this, &VD, "models/collectibles/coll_leaf.gltf", GLTF);
			M_potion1.init(this, &VD, "models/collectibles/coll_potion1.gltf", GLTF);
			M_potion2.init(this, &VD, "models/collectibles/coll_potion2.gltf", GLTF);

			M_chair.init(this, &VD, "models/kitchen/kitchen_chair.gltf", GLTF);
			M_fridge.init(this, &VD, "models/kitchen/kitchen_fridge.gltf", GLTF);
			M_kitchen.init(this, &VD, "models/kitchen/kitchen_kitchen.gltf", GLTF);
			M_kitchentable.init(this, &VD, "models/kitchen/kitchen_table.gltf", GLTF);

			M_cauldron.init(this, &VD, "models/lair/lair_cauldron.gltf", GLTF);
			M_stonechair.init(this, &VD, "models/lair/lair_chair.gltf", GLTF);
			M_chest.init(this, &VD, "models/lair/lair_chest.gltf", GLTF);
			M_shelf1.init(this, &VD, "models/lair/lair_shelf1.gltf", GLTF);
			M_shelf2.init(this, &VD, "models/lair/lair_shelf2.gltf", GLTF);
			M_stonetable.init(this, &VD, "models/lair/lair_table.gltf", GLTF);
			M_steam.init(this, &VD, "models/lair/lair_plane.gltf", GLTF);

			M_sofa.init(this, &VD, "models/livingroom/livingroom_sofa.gltf", GLTF);
			M_table.init(this, &VD, "models/livingroom/livingroom_table.gltf", GLTF);
			M_tv.init(this, &VD, "models/livingroom/livingroom_tv.gltf", GLTF);

			M_cat.init(this, &VD, "models/other/cat.gltf", GLTF);
			M_floor.init(this, &VD, "models/other/floor.gltf", GLTF);
			M_walls.init(this, &VD, "models/other/walls.gltf", GLTF);

			M_skyBox.init(this, &VD_skyBox, "models/sky/SkyBoxCube.obj", OBJ);

			for (int i = 0; i < collectiblesBBs.size(); i++) {
				M_boundingBox.push_back(Model<VertexBoundingBox>());
				createBBModel(M_boundingBox[i].vertices, M_boundingBox[i].indices, &collectiblesBBs[i]);
				M_boundingBox[i].initMesh(this, &VD_boundingBox);
			}

			for (int i = 0; i < fornitureBBs.size(); i++) {
				M_boundingBox.push_back(Model<VertexBoundingBox>());
				createBBModel(M_boundingBox[i + COLLECTIBLES_NUM].vertices, M_boundingBox[i + COLLECTIBLES_NUM].indices, &fornitureBBs[i]);
				M_boundingBox[i + COLLECTIBLES_NUM].initMesh(this, &VD_boundingBox);
			}

			M_boundingBox.push_back(Model<VertexBoundingBox>());
			createBBModel(M_boundingBox[COLLECTIBLES_NUM + fornitureBBs.size()].vertices, M_boundingBox[COLLECTIBLES_NUM + fornitureBBs.size()].indices, &catBox);
			M_boundingBox[COLLECTIBLES_NUM + fornitureBBs.size()].initMesh(this, &VD_boundingBox);

			// Create HUD timer
			glm::vec2 anchor = glm::vec2(0.8f, -0.95f);
			float w = 0.15f;
			float h = w * Ar;		// Respect the aspect ratio since it is a square pic
			for (int i = 0; i < 5; i++) {
				M_timer[i].vertices = { {{anchor.x, anchor.y}, {0.0f,0.0f}}, {{anchor.x, anchor.y + h}, {0.0f,1.0f}},
										{{anchor.x + w, anchor.y}, {1.0f,0.0f}}, {{ anchor.x + w, anchor.y + h}, {1.0f,1.0f}} };
				M_timer[i].indices = { 0, 1, 2,    1, 2, 3 };
				M_timer[i].initMesh(this, &VD_overlay);
			}

			// Create HUD scroll
			anchor = glm::vec2(-1.f, -0.9f);
			w = 0.2f;
			h = 1.8f;
			M_scroll.vertices = { {{anchor.x, anchor.y}, {0.0f,0.0f}}, {{anchor.x, anchor.y + h}, {0.0f,1.0f}},
								{{anchor.x + w, anchor.y}, {1.0f,0.0f}}, {{ anchor.x + w, anchor.y + h}, {1.0f,1.0f}} };
			M_scroll.indices = { 0, 1, 2,    1, 2, 3 };
			M_scroll.initMesh(this, &VD_overlay);

			// Create HUD collectibles
			anchor = glm::vec2(-1.f, -0.92f);
			w = 0.15f;
			h = w * Ar;
			for (int i = 0; i < COLLECTIBLES_NUM; i++) {
				anchor = anchor + (glm::vec2(0.f, 0.2f));

				M_collectibles[i].vertices = { {{anchor.x, anchor.y}, {0.0f,0.0f}}, {{anchor.x, anchor.y + h}, {0.0f,1.0f}},
											{{anchor.x + w, anchor.y}, {1.0f,0.0f}}, {{ anchor.x + w, anchor.y + h}, {1.0f,1.0f}} };
				M_collectibles[i].indices = { 0, 1, 2,    1, 2, 3 };
				M_collectibles[i].initMesh(this, &VD_overlay);
			}


			// Create the textures
			// The second parameter is the file name
			T_textures.init(this, "textures/textures.png");
			T_closet.init(this, "textures/closet.png");
			T_eye.init(this, "textures/eye_texture.jpg");
			T_feather.init(this, "textures/fabrics_0038_color_1k.jpg");
			T_steam.init(this, "textures/steam.png");

			T_skyBox.init(this, "textures/texture.jpg");

			T_timer[0].init(this, "textures/HUD/timer_100.png");
			T_timer[1].init(this, "textures/HUD/timer_75.png");
			T_timer[2].init(this, "textures/HUD/timer_50.png");
			T_timer[3].init(this, "textures/HUD/timer_25.png");
			T_timer[4].init(this, "textures/HUD/timer_0.png");

			T_scroll.init(this, "textures/HUD/scroll.png");

			T_collectibles[collectiblesHUD["crystal"]].init(this, "textures/HUD/coll_crystal.png");
			T_collectibles[collectiblesHUD["eye"]].init(this, "textures/HUD/coll_eye.png");
			T_collectibles[collectiblesHUD["feather"]].init(this, "textures/HUD/coll_feather.png");
			T_collectibles[collectiblesHUD["leaf"]].init(this, "textures/HUD/coll_leaf.png");
			T_collectibles[collectiblesHUD["potion1"]].init(this, "textures/HUD/coll_potion1.png");
			T_collectibles[collectiblesHUD["potion2"]].init(this, "textures/HUD/coll_potion2.png");
			T_collectibles[collectiblesHUD["bone"]].init(this, "textures/HUD/coll_bone.png");
	}

	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		P.create();
		P_skyBox.create();
		P_steam.create();
		P_boundingBox.create();
		P_overlay.create();

		DS_skyBox.init(this, &DSL_skyBox, {
						{0, UNIFORM, sizeof(SkyBoxUniformBufferObject), nullptr},
						{1, TEXTURE, 0, &T_skyBox},
						{2, UNIFORM, sizeof(float), nullptr}
			});

		// Here you define the data set
		DS_bed.init(this, &DSL, {
			// the second parameter, is a pointer to the Uniform Set Layout of this set
			// the last parameter is an array, with one element per binding of the set.
			// first  elmenet : the binding number
			// second element : UNIFORM or TEXTURE (an enum) depending on the type
			// third  element : only for UNIFORMs, the size of the corresponding C++ object. For texture, just put 0
			// fourth element : only for TEXTUREs, the pointer to the corresponding texture object. For uniforms, use nullptr
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &T_textures},
						{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
						{3, UNIFORM, sizeof(glm::vec3), nullptr}  // Emissive color binding
			});
		DS_closet.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_closet},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_nighttable.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});

		DS_bathtub.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});

		DS_bidet.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_sink.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_toilet.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});

		DS_bone.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_crystal.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_eye.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_eye},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_feather.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_feather},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_leaf.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_potion1.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_potion2.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});

		DS_chair.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_fridge.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_kitchen.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_kitchentable.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});

		DS_cauldron.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_stonechair.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_chest.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_shelf1.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_shelf2.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_stonetable.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_steam.init(this, &DSL_steam, {
					{0, UNIFORM, sizeof(SteamUniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_steam}
			});

		DS_sofa.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_table.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_tv.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});

		DS_cat.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_floor.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		DS_walls.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr},
					{3, UNIFORM, sizeof(glm::vec3), nullptr}
			});
		
		for (int i = 0; i < collectiblesBBs.size() + fornitureBBs.size() + 1; i++) {
			DS_boundingBox.push_back(DescriptorSet());
			DS_boundingBox[i].init(this, &DSL_boundingBox, {
						{0, UNIFORM, sizeof(BoundingBoxUniformBlock), nullptr},
				});
		}

		for (int i = 0; i < 5; i++) {
			DS_timer[i].init(this, &DSL_overlay, {
					{0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
					{1, TEXTURE, 0, &T_timer[i]}
				});
		}

		DS_scroll.init(this, &DSL_overlay, {
				{0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
				{1, TEXTURE, 0, &T_scroll}
			});

		for (int i = 0; i < COLLECTIBLES_NUM; i++) {
			DS_collectibles[i].init(this, &DSL_overlay, {
					{0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
					{1, TEXTURE, 0, &T_collectibles[i]}
				});
		}
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		P.cleanup();
		P_skyBox.cleanup();
		P_steam.cleanup();
		P_boundingBox.cleanup();
		P_overlay.cleanup();

		// Cleanup datasets
		DS_bathtub.cleanup();
		DS_bidet.cleanup();
		DS_sink.cleanup();
		DS_toilet.cleanup();

		DS_bed.cleanup();
		DS_closet.cleanup();
		DS_nighttable.cleanup();

		DS_bone.cleanup();
		DS_crystal.cleanup();
		DS_eye.cleanup();
		DS_feather.cleanup();
		DS_leaf.cleanup();
		DS_potion1.cleanup();
		DS_potion2.cleanup();

		DS_chair.cleanup();
		DS_fridge.cleanup();
		DS_kitchen.cleanup();
		DS_kitchentable.cleanup();

		DS_cauldron.cleanup();
		DS_stonechair.cleanup();
		DS_chest.cleanup();
		DS_shelf1.cleanup();
		DS_shelf2.cleanup();
		DS_stonetable.cleanup();
		DS_steam.cleanup();

		DS_sofa.cleanup();
		DS_table.cleanup();
		DS_tv.cleanup();

		DS_cat.cleanup();
		DS_floor.cleanup();
		DS_walls.cleanup();

		DS_skyBox.cleanup();

		for (int i = 0; i < collectiblesBBs.size() + fornitureBBs.size() + 1; i++) {
			DS_boundingBox[i].cleanup();
		}

		for (int i = 0; i < 5; i++) {
			DS_timer[i].cleanup();
		}

		DS_scroll.cleanup();

		for (int i = 0; i < COLLECTIBLES_NUM; i++) {
			DS_collectibles[i].cleanup();
		}
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() {
		// Cleanup textures
		T_textures.cleanup();
		T_eye.cleanup();
		T_closet.cleanup();
		T_feather.cleanup();
		T_steam.cleanup();

		T_skyBox.cleanup();

		for (int i = 0; i < 5; i++) {
			T_timer[i].cleanup();
		}

		T_scroll.cleanup();

		for (int i = 0; i < COLLECTIBLES_NUM; i++) {
			T_collectibles[i].cleanup();
		}

		// Cleanup models
		M_bathtub.cleanup();
		M_bidet.cleanup();
		M_sink.cleanup();
		M_toilet.cleanup();

		M_bed.cleanup();
		M_closet.cleanup();
		M_nighttable.cleanup();

		M_bone.cleanup();
		M_crystal.cleanup();
		M_eye.cleanup();
		M_feather.cleanup();
		M_leaf.cleanup();
		M_potion1.cleanup();
		M_potion2.cleanup();

		M_chair.cleanup();
		M_fridge.cleanup();
		M_kitchen.cleanup();
		M_kitchentable.cleanup();

		M_cauldron.cleanup();
		M_stonechair.cleanup();
		M_chest.cleanup();
		M_shelf1.cleanup();
		M_shelf2.cleanup();
		M_stonetable.cleanup();
		M_steam.cleanup();

		M_sofa.cleanup();
		M_table.cleanup();
		M_tv.cleanup();

		M_cat.cleanup();
		M_floor.cleanup();
		M_walls.cleanup();

		M_skyBox.cleanup();

		for (int i = 0; i < collectiblesBBs.size() + fornitureBBs.size() + 1; i++) {
			M_boundingBox[i].cleanup();
		}
		
		for (int i = 0; i < 5; i++) {
			M_timer[i].cleanup();
		}

		M_scroll.cleanup();

		for (int i = 0; i < COLLECTIBLES_NUM; i++) {
			M_collectibles[i].cleanup();
		}

		// Cleanup descriptor set layouts
		DSL.cleanup();
		DSL_skyBox.cleanup();
		DSL_steam.cleanup();
		DSL_boundingBox.cleanup();
		DSL_overlay.cleanup();

		// Destroies the pipelines
		P.destroy();
		P_skyBox.destroy();
		P_steam.destroy();
		P_boundingBox.destroy();
		P_overlay.destroy();
	}

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures

	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		// binds the pipeline
		P.bind(commandBuffer);
		// For a pipeline object, this command binds the corresponing pipeline to the command buffer passed in its parameter
		// binds the data set
		DS_bed.bind(commandBuffer, P, 0, currentImage);
		// For a Dataset object, this command binds the corresponing dataset
		// to the command buffer and pipeline passed in its first and second parameters.
		// The third parameter is the number of the set being bound
		// As described in the Vulkan tutorial, a different dataset is required for each image in the swap chain.
		// This is done automatically in file Starter.hpp, however the command here needs also the index
		// of the current image in the swap chain, passed in its last parameter
		// binds the model
		M_bed.bind(commandBuffer);
		// For a Model object, this command binds the corresponing index and vertex buffer
		// to the command buffer passed in its parameter
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(M_bed.indices.size()), 1, 0, 0, 0);
		// the second parameter is the number of indexes to be drawn. For a Model object,
		// this can be retrieved with the .indices.size() method.a

		DS_closet.bind(commandBuffer, P, 0, currentImage);
		M_closet.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_closet.indices.size()), 1, 0, 0, 0);

		DS_nighttable.bind(commandBuffer, P, 0, currentImage);
		M_nighttable.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_nighttable.indices.size()), 1, 0, 0, 0);

		DS_bathtub.bind(commandBuffer, P, 0, currentImage);
		M_bathtub.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_bathtub.indices.size()), 1, 0, 0, 0);

		DS_bidet.bind(commandBuffer, P, 0, currentImage);
		M_bidet.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_bidet.indices.size()), 1, 0, 0, 0);

		DS_sink.bind(commandBuffer, P, 0, currentImage);
		M_sink.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_sink.indices.size()), 1, 0, 0, 0);

		DS_toilet.bind(commandBuffer, P, 0, currentImage);
		M_toilet.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_toilet.indices.size()), 1, 0, 0, 0);

		DS_bone.bind(commandBuffer, P, 0, currentImage);
		M_bone.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_bone.indices.size()), 1, 0, 0, 0);

		DS_crystal.bind(commandBuffer, P, 0, currentImage);
		M_crystal.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_crystal.indices.size()), 1, 0, 0, 0);

		DS_eye.bind(commandBuffer, P, 0, currentImage);
		M_eye.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_eye.indices.size()), 1, 0, 0, 0);

		DS_feather.bind(commandBuffer, P, 0, currentImage);
		M_feather.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_feather.indices.size()), 1, 0, 0, 0);

		DS_leaf.bind(commandBuffer, P, 0, currentImage);
		M_leaf.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_leaf.indices.size()), 1, 0, 0, 0);

		DS_potion1.bind(commandBuffer, P, 0, currentImage);
		M_potion1.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_potion1.indices.size()), 1, 0, 0, 0);

		DS_potion2.bind(commandBuffer, P, 0, currentImage);
		M_potion2.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_potion2.indices.size()), 1, 0, 0, 0);

		DS_chair.bind(commandBuffer, P, 0, currentImage);
		M_chair.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_chair.indices.size()), 1, 0, 0, 0);

		DS_fridge.bind(commandBuffer, P, 0, currentImage);
		M_fridge.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_fridge.indices.size()), 1, 0, 0, 0);

		DS_kitchen.bind(commandBuffer, P, 0, currentImage);
		M_kitchen.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_kitchen.indices.size()), 1, 0, 0, 0);

		DS_kitchentable.bind(commandBuffer, P, 0, currentImage);
		M_kitchentable.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_kitchentable.indices.size()), 1, 0, 0, 0);

		DS_cauldron.bind(commandBuffer, P, 0, currentImage);
		M_cauldron.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_cauldron.indices.size()), 1, 0, 0, 0);

		DS_stonechair.bind(commandBuffer, P, 0, currentImage);
		M_stonechair.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_stonechair.indices.size()), 1, 0, 0, 0);

		DS_chest.bind(commandBuffer, P, 0, currentImage);
		M_chest.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_chest.indices.size()), 1, 0, 0, 0);

		DS_shelf1.bind(commandBuffer, P, 0, currentImage);
		M_shelf1.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_shelf1.indices.size()), 1, 0, 0, 0);

		DS_shelf2.bind(commandBuffer, P, 0, currentImage);
		M_shelf2.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_shelf2.indices.size()), 1, 0, 0, 0);

		DS_stonetable.bind(commandBuffer, P, 0, currentImage);
		M_stonetable.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_stonetable.indices.size()), 1, 0, 0, 0);

		DS_sofa.bind(commandBuffer, P, 0, currentImage);
		M_sofa.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_sofa.indices.size()), 1, 0, 0, 0);

		DS_table.bind(commandBuffer, P, 0, currentImage);
		M_table.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_table.indices.size()), 1, 0, 0, 0);

		DS_tv.bind(commandBuffer, P, 0, currentImage);
		M_tv.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_tv.indices.size()), 1, 0, 0, 0);

		DS_cat.bind(commandBuffer, P, 0, currentImage);
		M_cat.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_cat.indices.size()), 1, 0, 0, 0);

		DS_floor.bind(commandBuffer, P, 0, currentImage);
		M_floor.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_floor.indices.size()), 1, 0, 0, 0);

		DS_walls.bind(commandBuffer, P, 0, currentImage);
		M_walls.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_walls.indices.size()), 1, 0, 0, 0);

		P_skyBox.bind(commandBuffer);
		M_skyBox.bind(commandBuffer);
		DS_skyBox.bind(commandBuffer, P_skyBox, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_skyBox.indices.size()), 1, 0, 0, 0);

		P_steam.bind(commandBuffer);
		M_steam.bind(commandBuffer);
		DS_steam.bind(commandBuffer, P_steam, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_steam.indices.size()), 1, 0, 0, 0);

		P_boundingBox.bind(commandBuffer);
		for (int i = 0; i < collectiblesBBs.size() + fornitureBBs.size() + 1; i++) {
			M_boundingBox[i].bind(commandBuffer);
			DS_boundingBox[i].bind(commandBuffer, P_boundingBox, 0, currentImage);
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_boundingBox[i].indices.size()), 1, 0, 0, 0);
		}

		P_overlay.bind(commandBuffer);
		for (int i = 0; i < 5; i++) {
			M_timer[i].bind(commandBuffer);
			DS_timer[i].bind(commandBuffer, P_overlay, 0, currentImage);
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_timer[i].indices.size()), 1, 0, 0, 0);
		}

		M_scroll.bind(commandBuffer);
		DS_scroll.bind(commandBuffer, P_overlay, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_scroll.indices.size()), 1, 0, 0, 0);

		for (int i = 0; i < COLLECTIBLES_NUM; i++) {
			M_collectibles[i].bind(commandBuffer);
			DS_collectibles[i].bind(commandBuffer, P_overlay, 0, currentImage);
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(M_collectibles[i].indices.size()), 1, 0, 0, 0);
		}

	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		// Standard procedure to quit when the ESC key is pressed
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (glfwGetKey(window, GLFW_KEY_P) && (totalElapsedTime - lastPressTime > minimumPressDelay)) {
			DEBUG = !DEBUG;
			lastPressTime = totalElapsedTime;
		}

		if (glfwGetKey(window, GLFW_KEY_O) && (totalElapsedTime - lastPressTime > minimumPressDelay)) {
			OVERLAY = !OVERLAY;
			lastPressTime = totalElapsedTime;
		}


		// Integration with the timers and the controllers
		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool fire = false;
		getSixAxis(deltaT, m, r, fire);
		// getSixAxis() is defined in Starter.hpp in the base class.
		// It fills the float point variable passed in its first parameter with the time
		// since the last call to the procedure.
		// It fills vec3 in the second parameters, with three values in the -1,1 range corresponding
		// to motion (with left stick of the gamepad, or ASWD + RF keys on the keyboard)
		// It fills vec3 in the third parameters, with three values in the -1,1 range corresponding
		// to motion (with right stick of the gamepad, or Arrow keys + QE keys on the keyboard, or mouse)
		// If fills the last boolean variable with true if fire has been pressed:
		//          SPACE on the keyboard, A or B button on the Gamepad, Right mouse button

		// Check if game is over because time has run out
		totalElapsedTime += deltaT;
		remainingTime = GAME_DURATION - totalElapsedTime;

		if (totalElapsedTime >= GAME_DURATION) {
			// game over logic goes here
			// for now it just closes the window
			std::cout << "Game Over" << std::endl;
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		else if (static_cast<int>(remainingTime) != lastDisplayedTime) {
			std::cout << "Time remaining: " << static_cast<int>(remainingTime) << std::endl;
			lastDisplayedTime = static_cast<int>(remainingTime);
		}


		// Parameters for camera movement and rotation
		const float ROT_SPEED = glm::radians(90.0f);
		const float MOVE_SPEED = 10.0f;

		// Update camera yaw, pitch, and roll
		camYaw += ROT_SPEED * deltaT * r.y;
		camPitch -= ROT_SPEED * deltaT * r.x;
		camRoll -= ROT_SPEED * deltaT * r.z;
		camDist -= MOVE_SPEED * deltaT * m.y;

		// Limit the distance from the cat and the pitch to avoid gimbal lock
		camDist = glm::clamp(camDist, -1.0f, 4.0f);
		camPitch = glm::clamp(camPitch, glm::radians(-18.0f), glm::radians(5.0f));
		// camRoll = glm::clamp(camRoll, glm::radians(-10.0f), glm::radians(10.0f));

		// Camera movement + redefine forward and right vectors
		glm::vec3 ux = glm::rotate(glm::mat4(1.0f), camYaw, glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1);
		glm::vec3 uz = glm::rotate(glm::mat4(1.0f), camYaw, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1);
		glm::vec3 cameraForward = glm::normalize(glm::vec3(sin(camYaw), 0.0f, cos(camYaw)));
		glm::vec3 cameraRight = glm::normalize(glm::vec3(cos(camYaw), 0.0f, -sin(camYaw)));

		// Cat movement
		if ((m.x != 0) || (m.z != 0)) {
			// catPosition.x += m.x * MOVE_SPEED * deltaT;	// Move left/right
			//catPosition.y += m.y * MOVE_SPEED * deltaT;	// Move up/down - do not enable otherwise cat flies
			// catPosition.z -= m.z * MOVE_SPEED * deltaT;	// Move forward/backward
			catPosition -= cameraForward * m.z * MOVE_SPEED * deltaT;
			catPosition += cameraRight * m.x * MOVE_SPEED * deltaT;

			// Cat rotation based on the movement vector
			float targetYaw = atan2(m.z, m.x);
			targetYaw += glm::radians(-180.0f); // same as + 3.1416 / 2.0
			catYaw = glm::mix(catYaw, targetYaw + camYaw, deltaT * 6.0f);	// 6.0 is the damping factor
		}

		// Limit the cat's movement to the house
		catPosition.x = glm::clamp(catPosition.x, -12.2f, 12.2f);
		catPosition.z = glm::clamp(catPosition.z, -12.2f, 12.2f);

		glm::vec3 camTarget = catPosition + glm::vec3(glm::rotate(glm::mat4(1), Yaw, glm::vec3(0, 1, 0)) *
			glm::vec4(CamTargetDelta, 1));

		// Update the camera position relative to the cat's position
		//camPos = catPosition + MOVE_SPEED * m.x * ux * deltaT;
		//camPos = camPos + MOVE_SPEED * m.y * glm::vec3(0, 1, 0) * deltaT;
		//camPos = camPos + MOVE_SPEED * m.z * uz * deltaT;

		//camPos.y += 4.0f;
		//camPos.z += 3.0f;

		camPos = camTarget + glm::vec3(glm::rotate(glm::mat4(1), Yaw + camYaw, glm::vec3(0, 1, 0)) * glm::rotate(glm::mat4(1), -camPitch, glm::vec3(1, 0, 0)) *
			glm::vec4(0, 0, camDist, 1));

		// Parameters
		// Camera FOV-y, Near Plane and Far Plane
		// Set up the view and projection matrices
		const float FOVy = glm::radians(45.0f);
		const float nearPlane = 0.1f;
		const float farPlane = 300.0f;

		glm::mat4 M = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		M[1][1] *= -1;

		// View matrix for camera following the cat
		glm::mat4 Mv = glm::rotate(glm::mat4(1.0f), -camPitch, glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1.0f), -camYaw, glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1.0f), -camRoll, glm::vec3(0, 0, 1)) *
			glm::translate(glm::mat4(1.0f), -camPos);

		glm::mat4 ViewPrj = M * Mv;

		// Update rotation angle of the collectibles
		collectibleRotationAngle += collectibleRotationSpeed * deltaT;
		if (collectibleRotationAngle >= glm::two_pi<float>()) {
			collectibleRotationAngle -= glm::two_pi<float>();
		}


		GlobalUniformBufferObject gubo = {};
		// Set light properties
		gubo.lightPos[0] = glm::vec3(6.0f, 2.0f, 8.0f);		// position: kitchen
		gubo.lightColor[0] = glm::vec3(1.4f);				// color: white

		gubo.lightPos[1] = glm::vec3(-8.f, 2.0f, -8.f);		// position: witch lair
		gubo.lightColor[1] = glm::vec3(0.4f, 0.f, 0.8f);	// color: purple

		gubo.lightPos[2] = glm::vec3(-6.0f, 1.3f, -8.3f);	// position: witch lair - cauldron
		gubo.lightColor[2] = glm::vec3(0.02f, 0.07f, 0.02f);// color: green

		gubo.lightPos[3] = glm::vec3(-6.0f, 0.2f, -8.3f);	// position: witch lair - cauldron fire
		gubo.lightColor[3] = glm::vec3(0.14f, 0.08f, 0.f);	// color: orange

		gubo.lightPos[4] = glm::vec3(11.9f, 1.0f, -4.f);	// position: bedroom
		gubo.lightColor[4] = glm::vec3(0.6f, 0.5f, 0.f);	// color: yellow

		gubo.lightPos[5] = glm::vec3(-7.0f, 2.0f, 7.f);		// position: living room
		gubo.lightColor[5] = glm::vec3(0.04f, 0.06f, 0.02f);// color: green

		gubo.lightPos[6] = glm::vec3(0.f, 2.0f, -8.f);		// position: bathroom
		gubo.lightColor[6] = glm::vec3(0.06f, 0.03f, 0.f);	// color: orange


		for (int i = 0; i < LIGHTS_NUM; i++) {
			gubo.constant[i] = 1.0f;    // Typically 1.0 for no attenuation
			gubo.linear[i] = 0.09f;     // Small factor for linear attenuation
			gubo.quadratic[i] = 0.032f; // Even smaller factor for quadratic attenuation
		}
		
		gubo.eyePos = camPos; // Camera position

		// Sky Box UBO update
		SkyBoxUniformBufferObject sbubo{};
		sbubo.mvpMat = M * glm::mat4(glm::mat3(Mv));
		sbubo.time = totalElapsedTime;
		DS_skyBox.map(currentImage, &sbubo, sizeof(sbubo), 0);

		// Steam UBO update
		SteamUniformBufferObject subo = {};
		glm::mat4 World = glm::translate(glm::mat4(1.0f), cauldron.pos + glm::vec3(0, 1.7f, 0)) *		// Steam plane position - over the cauldron
			glm::rotate(glm::mat4(1.0f), camYaw, glm::vec3(0, 1, 0));									// Steam plane rotation - always face the camera
		subo.mvpMat = ViewPrj * World;
		subo.mMat = World;
		subo.nMat = glm::transpose(glm::inverse(World));
		subo.time = totalElapsedTime;
		DS_steam.map(currentImage, &subo, sizeof(subo), 0);

		// Overlays UBO updates
		if (OVERLAY) {
			// Timer
			if (remainingTime >= GAME_DURATION * 3 / 4) {
				UBO_timer[0].visible = 1.f;
				UBO_timer[1].visible = UBO_timer[2].visible = UBO_timer[3].visible = UBO_timer[4].visible = 0.f;
			}
			else if (remainingTime >= GAME_DURATION / 2 && remainingTime) {
				UBO_timer[1].visible = 1.f;
				UBO_timer[0].visible = UBO_timer[2].visible = UBO_timer[3].visible = UBO_timer[4].visible = 0.f;
			}
			else if (remainingTime >= GAME_DURATION / 4) {
				UBO_timer[2].visible = 1.f;
				UBO_timer[0].visible = UBO_timer[1].visible = UBO_timer[3].visible = UBO_timer[4].visible = 0.f;
			}
			else if (remainingTime > 0) {
				UBO_timer[3].visible = 1.f;
				UBO_timer[0].visible = UBO_timer[1].visible = UBO_timer[2].visible = UBO_timer[4].visible = 0.f;
			}
			else {
				UBO_timer[4].visible = 1.f;
				UBO_timer[0].visible = UBO_timer[1].visible = UBO_timer[2].visible = UBO_timer[3].visible = 0.f;
			}

			// Scroll
			UBO_scroll.visible = 1.f;

			// Collectibles
			UBO_collectibles[collectiblesHUD["crystal"]].visible = !collectiblesMap["crystal"] ? 1.f : 0.f;
			UBO_collectibles[collectiblesHUD["eye"]].visible = !collectiblesMap["eye"] ? 1.f : 0.f;
			UBO_collectibles[collectiblesHUD["feather"]].visible = !collectiblesMap["feather"] ? 1.f : 0.f;
			UBO_collectibles[collectiblesHUD["leaf"]].visible = !collectiblesMap["leaf"] ? 1.f : 0.f;
			UBO_collectibles[collectiblesHUD["potion1"]].visible = !collectiblesMap["potion1"] ? 1.f : 0.f;
			UBO_collectibles[collectiblesHUD["potion2"]].visible = !collectiblesMap["potion2"] ? 1.f : 0.f;
			UBO_collectibles[collectiblesHUD["bone"]].visible = !collectiblesMap["bone"] ? 1.f : 0.f;
		}
		else {
			UBO_timer[0].visible = UBO_timer[1].visible = UBO_timer[2].visible = UBO_timer[3].visible = UBO_timer[4].visible = 0.f;
			UBO_scroll.visible = 0.f;
			for (int i = 0; i < COLLECTIBLES_NUM; i++) {
				UBO_collectibles[i].visible = 0.f;
			}
		}

		for (int i = 0; i < 5; i++) {
			DS_timer[i].map(currentImage, &UBO_timer[i], sizeof(UBO_timer[i]), 0);
		}
		DS_scroll.map(currentImage, &UBO_scroll, sizeof(UBO_scroll), 0);

		for (int i = 0; i < COLLECTIBLES_NUM; i++) {
			DS_collectibles[i].map(currentImage, &UBO_collectibles[i], sizeof(UBO_collectibles[i]), 0);
		}



		// Placing cat
		placeEntity(UBO_cat, gubo, catPosition, glm::vec3(0, catYaw, 0), glm::vec3(1.f), glm::vec3(0.0f), ViewPrj, DS_cat, currentImage, DEBUG, 15);
		catBox = BoundingBox("cat", catPosition, catDimensions);

		// House
		placeEntity(UBO_floor, gubo, houseFloor.pos, houseFloor.rot, houseFloor.scale, glm::vec3(0.0f), ViewPrj, DS_floor, currentImage, false);
		placeEntity(UBO_walls, gubo, walls.pos, walls.rot, walls.scale, glm::vec3(0.0f), ViewPrj, DS_walls, currentImage, false);

		// Bedroom
		placeEntity(UBO_closet, gubo, closet.pos, closet.rot, closet.scale, glm::vec3(0.0f), ViewPrj, DS_closet, currentImage, DEBUG, 8);
		placeEntity(UBO_bed, gubo, bed.pos, bed.rot, bed.scale, glm::vec3(0.0f), ViewPrj, DS_bed, currentImage, DEBUG, 9);
		placeEntity(UBO_nightTable, gubo, nightTable.pos, nightTable.rot, nightTable.scale, glm::vec3(0.0f), ViewPrj, DS_nighttable, currentImage, DEBUG, 10);

		// Kitchen
		placeEntity(UBO_kitchen, gubo, kitchen.pos, kitchen.rot, kitchen.scale, glm::vec3(0.0f), ViewPrj, DS_kitchen, currentImage, DEBUG, 14);
		placeEntity(UBO_fridge, gubo, fridge.pos, fridge.rot, fridge.scale, glm::vec3(0.0f), ViewPrj, DS_fridge, currentImage, DEBUG, 13);
		placeEntity(UBO_kitchenTable, gubo, kitchenTable.pos, kitchenTable.rot, kitchenTable.scale, glm::vec3(0.0f), ViewPrj, DS_kitchentable, currentImage, false);
		placeEntity(UBO_chair, gubo, chair.pos, chair.rot, chair.scale, glm::vec3(0.0f), ViewPrj, DS_chair, currentImage, false);

		// Living room
		placeEntity(UBO_sofa, gubo, sofa.pos, sofa.rot, sofa.scale, glm::vec3(0.0f), ViewPrj, DS_sofa, currentImage, DEBUG, 12);
		placeEntity(UBO_table, gubo, table.pos, table.rot, table.scale, glm::vec3(0.0f), ViewPrj, DS_table, currentImage, false);
		placeEntity(UBO_tv, gubo, tv.pos, tv.rot, tv.scale, glm::vec3(0.0f), ViewPrj, DS_tv, currentImage, false);

		// Witch lair
		placeEntity(UBO_chest, gubo, chest.pos, chest.rot, chest.scale, glm::vec3(0.0f), ViewPrj, DS_chest, currentImage, DEBUG, 11);
		placeEntity(UBO_stoneTable, gubo, stoneTable.pos, stoneTable.rot, stoneTable.scale, glm::vec3(0.0f), ViewPrj, DS_stonetable, currentImage, false);
		placeEntity(UBO_stoneChair, gubo, stoneChair.pos, stoneChair.rot, stoneChair.scale, glm::vec3(0.0f), ViewPrj, DS_stonechair, currentImage, false);
		placeEntity(UBO_cauldron, gubo, cauldron.pos, cauldron.rot, cauldron.scale, glm::vec3(0.0f), ViewPrj, DS_cauldron, currentImage, false);
		placeEntity(UBO_shelf1, gubo, shelf1.pos, shelf1.rot, shelf1.scale, glm::vec3(0.0f), ViewPrj, DS_shelf1, currentImage, false);
		placeEntity(UBO_shelf2, gubo, shelf2.pos, shelf2.rot, shelf2.scale, glm::vec3(0.0f), ViewPrj, DS_shelf2, currentImage, false);

		// Bathroom
		placeEntity(UBO_bathtub, gubo, bathtub.pos, bathtub.rot, bathtub.scale, glm::vec3(0.0f), ViewPrj, DS_bathtub, currentImage, DEBUG, 7);
		placeEntity(UBO_toilet, gubo, toilet.pos, toilet.rot, toilet.scale, glm::vec3(0.0f), ViewPrj, DS_toilet, currentImage, false);
		placeEntity(UBO_bidet, gubo, bidet.pos, bidet.rot, bidet.scale, glm::vec3(0.0f), ViewPrj, DS_bidet, currentImage, false);
		placeEntity(UBO_sink, gubo, sink.pos, sink.rot, sink.scale, glm::vec3(0.0f), ViewPrj, DS_sink, currentImage, false);


		// Collectibles
		if (!collectiblesMap["crystal"]) {
			placeEntity(UBO_crystal, gubo, collectiblesRandomPosition[0], glm::vec3(0, collectibleRotationAngle, 0), glm::vec3(1.0f), glm::vec3(1.0f), ViewPrj, DS_crystal, currentImage, DEBUG, 0);
		} else {
			removeCollectible(UBO_crystal, gubo, ViewPrj, DS_crystal, currentImage, 0);	// it actually scales to zero -> not efficient
		}
		if (!collectiblesMap["eye"]) {
			placeEntity(UBO_eye, gubo, collectiblesRandomPosition[1], glm::vec3(0, collectibleRotationAngle, 0), glm::vec3(1.0f), glm::vec3(2.0f), ViewPrj, DS_eye, currentImage, DEBUG, 1);
		} else {
			removeCollectible(UBO_eye, gubo, ViewPrj, DS_eye, currentImage, 1);
		}
		if (!collectiblesMap["feather"]) {
			placeEntity(UBO_feather, gubo, collectiblesRandomPosition[2], glm::vec3(0, collectibleRotationAngle, 0), glm::vec3(0.7f), glm::vec3(2.0f), ViewPrj, DS_feather, currentImage, DEBUG, 2);
		} else {
			removeCollectible(UBO_feather, gubo, ViewPrj, DS_feather, currentImage, 2);
		}
		if (!collectiblesMap["leaf"]) {
			placeEntity(UBO_leaf, gubo, collectiblesRandomPosition[3], glm::vec3(0, collectibleRotationAngle, 0), glm::vec3(0.7f), glm::vec3(3.0f), ViewPrj, DS_leaf, currentImage, DEBUG, 3);
		} else {
			removeCollectible(UBO_leaf, gubo, ViewPrj, DS_leaf, currentImage, 3);
		}
		if (!collectiblesMap["potion1"]) {
			placeEntity(UBO_potion1, gubo, collectiblesRandomPosition[4], glm::vec3(0, collectibleRotationAngle, 0), glm::vec3(0.7f), glm::vec3(3.0f), ViewPrj, DS_potion1, currentImage, DEBUG, 4);
		} else {
			removeCollectible(UBO_potion1, gubo, ViewPrj, DS_potion1, currentImage, 4);
		}
		if (!collectiblesMap["potion2"]) {
			placeEntity(UBO_potion2, gubo, collectiblesRandomPosition[5], glm::vec3(0, collectibleRotationAngle, 0), glm::vec3(0.7f), glm::vec3(3.0f), ViewPrj, DS_potion2, currentImage, DEBUG, 5);
		} else {
			removeCollectible(UBO_potion2, gubo, ViewPrj, DS_potion2, currentImage, 5);
		}
		if (!collectiblesMap["bone"]) {
			placeEntity(UBO_bone, gubo, collectiblesRandomPosition[6], glm::vec3(0, collectibleRotationAngle, 0), glm::vec3(1.0f), glm::vec3(1.0f), ViewPrj, DS_bone, currentImage, DEBUG, 6);
		} else {
			removeCollectible(UBO_bone, gubo, ViewPrj, DS_bone, currentImage, 6);
		}

		// Check for collisions with the collectibles
		for (int i = 0; i < collectiblesBBs.size(); i++) {
			if (catBox.intersects(collectiblesBBs[i])) {
				collectiblesMap[collectiblesBBs[i].getName()] = true;
				std::cout << "Collected " << collectiblesBBs[i].getName() << "!" << std::endl;

				int collectibleIndex = collectiblesHUD[collectiblesBBs[i].getName()];

				UBO_collectibles[collectibleIndex].visible = 0.f;
				DS_collectibles[collectibleIndex].map(currentImage, &UBO_collectibles[collectibleIndex], sizeof(UBO_collectibles[collectibleIndex]), 0);

				if (collectiblesMap["crystal"] && collectiblesMap["eye"] && collectiblesMap["feather"] &&
					collectiblesMap["leaf"] && collectiblesMap["potion1"] && collectiblesMap["potion2"] && collectiblesMap["bone"]) {
					std::cout << "\nALL COLLECTIBLES COLLECTED!" << std::endl;
					// win logic goes here
					std::cout << "You won!" << std::endl;
					glfwSetWindowShouldClose(window, GL_TRUE);
				}
			}
		}

		for (int j = 0; j < fornitureBBs.size(); j++) {
			if (catBox.intersects(fornitureBBs[j])) {
				catPosition += cameraForward * m.z * MOVE_SPEED * deltaT;
				//catPosition.y -= m.y * MOVE_SPEED * deltaT;
				catPosition -= cameraRight * m.x * MOVE_SPEED * deltaT;

				std::cout << "Collision with " << fornitureBBs[j].getName() << std::endl;
			}
		}
	}

	void placeEntity(UniformBufferObject ubo, GlobalUniformBufferObject gubo, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale,
					glm::vec3 emissiveColor, glm::mat4 ViewPrj, DescriptorSet ds, int currentImage, bool hasBoundingBox, int id = 0) {

		glm::mat4 World = glm::translate(glm::mat4(1), position) *
			glm::rotate(glm::mat4(1), rotation.x, glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1), rotation.z, glm::vec3(0, 0, 1)) *
			glm::scale(glm::mat4(1), scale);
		ubo.mvpMat = ViewPrj * World;
		ubo.mMat = World;
		ubo.nMat = glm::transpose(glm::inverse(World));

		drawBoundingBox(hasBoundingBox, position, rotation, scale, ViewPrj, UBO_boundingBox[id], DS_boundingBox[id], currentImage);

		ds.map(currentImage, &ubo, sizeof(ubo), 0);
		ds.map(currentImage, &gubo, sizeof(gubo), 2);
		// the .map() method of a DataSet object, requires the current image of the swap chain as first parameter
		// the second parameter is the pointer to the C++ data structure to transfer to the GPU
		// the third parameter is its size
		// the fourth parameter is the location inside the descriptor set of this uniform block

		ds.map(currentImage, &emissiveColor, sizeof(emissiveColor), 3);
	}

	void removeCollectible(UniformBufferObject ubo, GlobalUniformBufferObject gubo, glm::mat4 ViewPrj, DescriptorSet ds, int currentImage, int id) {
		glm::mat4 World = glm::mat4(0.f);
		ubo.mvpMat = ViewPrj * World;
		ubo.mMat = World;
		ubo.nMat = glm::transpose(glm::inverse(World));

		ds.map(currentImage, &ubo, sizeof(ubo), 0);
		ds.map(currentImage, &gubo, sizeof(gubo), 2);

		// update bounding box matrices
		UBO_boundingBox[id].mvpMat = ViewPrj * World;
		UBO_boundingBox[id].mMat = World;
		UBO_boundingBox[id].nMat = glm::inverse(glm::transpose(World));
		DS_boundingBox[id].map(currentImage, &UBO_boundingBox[id], sizeof(UBO_boundingBox[id]), 0);

		// erase the bounding box from the map
		collectiblesBBs[id].erase();
	}

};


// This is the main: probably you do not need to touch this!
int main() {
    PurrfectPotion app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}