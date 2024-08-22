// This has been adapted from the Vulkan tutorial
#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include "Starter.hpp"
#include "BoundingBox.hpp"

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
	alignas(16) glm::vec3 lightDir;
	alignas(16) glm::vec4 lightColor;
	alignas(16) glm::vec3 eyePos;
};


// The vertices data structures
// Example
struct Vertex {
	glm::vec3 pos;
	glm::vec2 UV;
	glm::vec3 norm;
};



// MAIN ! 
class MeshLoader : public BaseProject {
	protected:

	// Current aspect ratio (used by the callback that resized the window
	float Ar;

	// Other application parameters
	glm::vec3 camPos = glm::vec3(0.0, 1.5, 7.0);
	float camYaw = glm::radians(90.0f);
	float camPitch = glm::radians(-30.0f);
	float camRoll = 0.0f;
	float camDist = 1.0f;
	const glm::vec3 CamTargetDelta = glm::vec3(0, 10, 0);
	//const glm::vec3 Cam1stPos = glm::vec3(0.49061f, 2.07f, 2.7445f);
	float Yaw = 0.0f;
	// Rotation angle for the cube
	float collectibleRotationAngle = 0.0f;
	// Rotation speed in radians per second
	const float collectibleRotationSpeed = glm::radians(45.0f);  // 45 degrees per second

	// Cat initial position
	glm::vec3 catPosition = glm::vec3(6.0f, 0.0f, 0.0f);
	// Cat initial orientation
	float catYaw = glm::radians(270.0f);

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSL;

	// Vertex formats
	VertexDescriptor VD;

	// Pipelines [Shader couples]
	Pipeline P;

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
	Model<Vertex> M_cat, M_floor, M_walls;

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
	DescriptorSet DS_cat, DS_floor, DS_walls;

	// Textures
	Texture T_textures, T_eye, T_closet, T_feather;
	
	// C++ storage for uniform variables
	// Bathroom
	UniformBufferObject UBO_bathtub, UBO_bidet, UBO_sink, UBO_toilet;
	// Bedroom
	UniformBufferObject UBO_bed, UBO_closet, UBO_nighttable;
	// Collectibles
	UniformBufferObject UBO_bone, UBO_crystal, UBO_eye, UBO_feather, UBO_leaf, UBO_potion1, UBO_potion2;
	// Kitchen
	UniformBufferObject UBO_chair, UBO_fridge, UBO_kitchen, UBO_kitchentable;
	// Lair
	UniformBufferObject UBO_cauldron, UBO_stonechair, UBO_chest, UBO_shelf1, UBO_shelf2, UBO_stonetable;
	// Living room
	UniformBufferObject UBO_sofa, UBO_table, UBO_tv;
	// Other
	UniformBufferObject UBO_cat, UBO_floor, UBO_walls;


	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "Mesh Loader";
    	windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 100; //30
		texturesInPool = 50;	//5
		setsInPool = 100;	//30
		
		Ar = (float)windowWidth / (float)windowHeight;
	}
	
	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		std::cout << "Window resized to: " << w << " x " << h << "\n";
		Ar = (float)w / (float)h;
	}
	
	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() {
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
					{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
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

		// Pipelines [Shader couples]
		// The second parameter is the pointer to the vertex definition
		// Third and fourth parameters are respectively the vertex and fragment shaders
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		P.init(this, &VD, "shaders/ShaderVert.spv", "shaders/ShaderFrag.spv", {&DSL});
		P.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);

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

		M_sofa.init(this, &VD, "models/livingroom/livingroom_sofa.gltf", GLTF);
		M_table.init(this, &VD, "models/livingroom/livingroom_table.gltf", GLTF);
		M_tv.init(this, &VD, "models/livingroom/livingroom_tv.gltf", GLTF);

		M_cat.init(this, &VD, "models/other/cat.gltf", GLTF);
		M_floor.init(this, &VD, "models/other/floor.gltf", GLTF);
		M_walls.init(this, &VD, "models/other/walls.gltf", GLTF);
		

		// Create the textures
		// The second parameter is the file name
		T_textures.init(this, "textures/textures.png");
		T_closet.init(this, "textures/closet.png");
		T_eye.init(this, "textures/eye_texture.jpg");
		T_feather.init(this, "textures/fabrics_0038_color_1k.jpg");
	}
	
	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		P.create();

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
						{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_closet.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_closet},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_nighttable.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});

		DS_bathtub.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});

		DS_bidet.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_sink.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_toilet.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});

		DS_bone.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_crystal.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_eye.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_eye},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_feather.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_feather},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_leaf.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_potion1.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_potion2.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});

		DS_chair.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_fridge.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_kitchen.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_kitchentable.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});

		DS_cauldron.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_stonechair.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_chest.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_shelf1.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_shelf2.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_stonetable.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});

		DS_sofa.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_table.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_tv.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});

		DS_cat.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_floor.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
		DS_walls.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T_textures},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
			});
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		P.cleanup();

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

		DS_sofa.cleanup();
		DS_table.cleanup();
		DS_tv.cleanup();

		DS_cat.cleanup();
		DS_floor.cleanup();
		DS_walls.cleanup();
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

		M_sofa.cleanup();
		M_table.cleanup();
		M_tv.cleanup();

		M_cat.cleanup();
		M_floor.cleanup();
		M_walls.cleanup();

		// Cleanup descriptor set layouts
		DSL.cleanup();

		// Destroies the pipelines
		P.destroy();
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

	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		// Standard procedure to quit when the ESC key is pressed
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
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


		// Parameters for camera movement and rotation
		const float ROT_SPEED = glm::radians(90.0f);
		const float MOVE_SPEED = 10.0f;

		// Update camera yaw, pitch, and roll
		camYaw += ROT_SPEED * deltaT * r.y;
		camPitch -= ROT_SPEED * deltaT * r.x;
		camRoll -= ROT_SPEED * deltaT * r.z;
		camDist -= MOVE_SPEED * deltaT * m.y;

		// Limit the pitch to avoid gimbal lock
		camPitch = camPitch < glm::radians(-90.0f) ? glm::radians(-90.0f) :
			(camPitch > glm::radians(90.0f) ? glm::radians(90.0f) : camPitch);

		camDist = (camDist < 7.0f ? 7.0f : (camDist > 15.0f ? 15.0f : camDist));

		// Camera movement + redefine forward and right vectors
		glm::vec3 ux = glm::rotate(glm::mat4(1.0f), camYaw, glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1);
		glm::vec3 uz = glm::rotate(glm::mat4(1.0f), camYaw, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1);
		glm::vec3 cameraForward = glm::normalize(glm::vec3(sin(camYaw), 0.0f, cos(camYaw)));
		glm::vec3 cameraRight = glm::normalize(glm::vec3(cos(camYaw), 0.0f, -sin(camYaw)));

		if ((m.x != 0) || (m.z != 0)) {
			// Cat movement
			// catPosition.x += m.x * MOVE_SPEED * deltaT;	// Move left/right
			//catPosition.y += m.y * MOVE_SPEED * deltaT;	// Move up/down - do not enable otherwise cat flies
			// catPosition.z -= m.z * MOVE_SPEED * deltaT;	// Move forward/backward
			catPosition -= cameraForward * m.z * MOVE_SPEED * deltaT;
			catPosition += cameraRight * m.x * MOVE_SPEED * deltaT;

			// Cat rotation based on the movement vector
			float targetYaw = atan2(m.z, m.x);
			targetYaw += glm::radians(90.0f); // same as + 3.1416 / 2.0
			// catYaw += (targetYaw - catYaw) * deltaT * 6.0f;	  // 6.0f is the damping factor
			// catYaw = glm::mix(catYaw, targetYaw, 0.1f);		  // alternative way to make the cat rotate smoothly
			catYaw = glm::mix(catYaw, targetYaw + camYaw, deltaT * 6.0f);

			// Check for collisions with the collectibles
			BoundingBox catBox = BoundingBox(catPosition, glm::vec3(0.13f, 0.6f, 1.0f));
			// Bounding boxes for the cat and the collectibles
			std::vector<BoundingBox> collectiblesBBs;
			collectiblesBBs.push_back(BoundingBox(glm::vec3(-3.f, 0.5f, 0.0f), glm::vec3(0.5f)));
			collectiblesBBs.push_back(BoundingBox(glm::vec3(-2.f, 0.5f, 0.0f), glm::vec3(0.5f)));
			collectiblesBBs.push_back(BoundingBox(glm::vec3(-1.f, 0.5f, 0.0f), glm::vec3(0.5f)));
			collectiblesBBs.push_back(BoundingBox(glm::vec3(0.f, 0.5f, 0.0f), glm::vec3(0.5f)));
			collectiblesBBs.push_back(BoundingBox(glm::vec3(1.f, 0.5f, 0.0f), glm::vec3(0.5f)));
			collectiblesBBs.push_back(BoundingBox(glm::vec3(2.f, 0.5f, 0.0f), glm::vec3(0.5f)));
			collectiblesBBs.push_back(BoundingBox(glm::vec3(3.f, 0.5f, 0.0f), glm::vec3(0.5f)));


			for (int i = 0; i < collectiblesBBs.size(); i++) {
				if (catBox.intersects(collectiblesBBs[i])) {
					catPosition += cameraForward * m.z * MOVE_SPEED * deltaT;
					//catPosition.y -= m.y * MOVE_SPEED * deltaT;
					catPosition -= cameraRight * m.x * MOVE_SPEED * deltaT;
					std::cout << "Collision with collectible " << i << std::endl;
				}
			}

			// Limit the cat's movement to the house
			catPosition.x = glm::clamp(catPosition.x, -7.2f, 7.2f);
			catPosition.z = glm::clamp(catPosition.z, -7.2f, 7.2f);

		}

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
		gubo.lightDir = glm::vec3(-1.0f, 1.0f, -1.0f); // Direction of the light
		gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // White light
		gubo.eyePos = camPos; // Camera position




		placeObject(UBO_cat, gubo, catPosition, glm::vec3(0, catYaw, 0), glm::vec3(1.0f), ViewPrj, DS_cat, currentImage);

		placeObject(UBO_floor, gubo, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_floor, currentImage);
		//placeObject(UBO_walls, gubo, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_walls, currentImage);

		// Bedroom
		placeObject(UBO_closet, gubo, glm::vec3(4.5f, 0.0f, -7.f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_closet, currentImage);
		placeObject(UBO_bed, gubo, glm::vec3(5.f, 0.0f, -4.f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_bed, currentImage);
		placeObject(UBO_nighttable, gubo, glm::vec3(6.9f, 0.0f, -2.f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_nighttable, currentImage);

		// Kitchen
		placeObject(UBO_kitchen, gubo, glm::vec3(4.5f, 0.0f, 6.6f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_kitchen, currentImage);
		placeObject(UBO_fridge, gubo, glm::vec3(0.4f, 0.0f, 6.8f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_fridge, currentImage);
		placeObject(UBO_kitchentable, gubo, glm::vec3(4.5f, 0.0f, 3.2f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_kitchentable, currentImage);
		placeObject(UBO_chair, gubo, glm::vec3(4.5f, 0.0f, 2.4f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_chair, currentImage);

		// Living room
		placeObject(UBO_sofa, gubo, glm::vec3(-6.5f, 0.0f, 5.7f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_sofa, currentImage);
		placeObject(UBO_table, gubo, glm::vec3(-6.2f, 0.0f, 2.f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_table, currentImage);
		placeObject(UBO_tv, gubo, glm::vec3(-4.f, 0.0f, 5.7f), glm::vec3(0, glm::radians(180.f), 0), glm::vec3(1.0f), ViewPrj, DS_tv, currentImage);

		// Witch lair
		placeObject(UBO_chest, gubo, glm::vec3(-5.f, 0.0f, -7.0f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_chest, currentImage);
		placeObject(UBO_stonetable, gubo, glm::vec3(-6.6f, 0.0f, -5.f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_stonetable, currentImage);
		placeObject(UBO_stonechair, gubo, glm::vec3(-5.8f, 0.0f, -5.f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_stonechair, currentImage);
		placeObject(UBO_cauldron, gubo, glm::vec3(-4.0f, 0.0f, -5.3f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_cauldron, currentImage);
		placeObject(UBO_shelf1, gubo, glm::vec3(-7.2f, 3.f, -5.5f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_shelf1, currentImage);
		placeObject(UBO_shelf2, gubo, glm::vec3(-5.f, 3.4f, -7.2f), glm::vec3(0, glm::radians(90.f), 0), glm::vec3(1.0f), ViewPrj, DS_shelf2, currentImage);

		// Bathroom
		placeObject(UBO_bathtub, gubo, glm::vec3(-0.5f, 0.0f, -6.8f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_bathtub, currentImage);
		placeObject(UBO_toilet, gubo, glm::vec3(0.5f, 0.0f, -5.5f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_toilet, currentImage);
		placeObject(UBO_bidet, gubo, glm::vec3(0.5f, 0.0f, -4.f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_bidet, currentImage);
		placeObject(UBO_sink, gubo, glm::vec3(-1.7f, 0.0f, -4.7f), glm::vec3(0.0f), glm::vec3(1.0f), ViewPrj, DS_sink, currentImage);

		// Collectibles
		placeObject(UBO_crystal, gubo, glm::vec3(-3.f, 0.5f, 0.0f), glm::vec3(0, collectibleRotationAngle, 0), glm::vec3(1.0f), ViewPrj, DS_crystal, currentImage);
		placeObject(UBO_eye, gubo, glm::vec3(-2.f, 0.5f, 0.0f), glm::vec3(0, collectibleRotationAngle, 0), glm::vec3(1.0f), ViewPrj, DS_eye, currentImage);
		placeObject(UBO_feather, gubo, glm::vec3(-1.f, 0.5f, 0.0f), glm::vec3(0, collectibleRotationAngle, 0), glm::vec3(1.0f), ViewPrj, DS_feather, currentImage);
		placeObject(UBO_leaf, gubo, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0, collectibleRotationAngle, 0), glm::vec3(1.0f), ViewPrj, DS_leaf, currentImage);
		placeObject(UBO_potion1, gubo, glm::vec3(1.f, 0.5f, 0.0f), glm::vec3(0, collectibleRotationAngle, 0), glm::vec3(1.0f), ViewPrj, DS_potion1, currentImage);
		placeObject(UBO_potion2, gubo, glm::vec3(2.f, 0.5f, 0.0f), glm::vec3(0, collectibleRotationAngle, 0), glm::vec3(1.0f), ViewPrj, DS_potion2, currentImage);
		placeObject(UBO_bone, gubo, glm::vec3(3.f, 0.5f, 0.0f), glm::vec3(0, collectibleRotationAngle, 0), glm::vec3(1.0f), ViewPrj, DS_bone, currentImage);
	}

	void placeObject(UniformBufferObject ubo, GlobalUniformBufferObject gubo, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::mat4 ViewPrj, DescriptorSet ds, int currentImage) {
		glm::mat4 World = glm::translate(glm::mat4(1), position) *
			glm::rotate(glm::mat4(1), rotation.x, glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1), rotation.z, glm::vec3(0, 0, 1)) *
			glm::scale(glm::mat4(1), scale);
		ubo.mvpMat = ViewPrj * World;
		ubo.mMat = World;
		ubo.nMat = glm::transpose(glm::inverse(World));
		ds.map(currentImage, &ubo, sizeof(ubo), 0);
		ds.map(currentImage, &gubo, sizeof(gubo), 2);
		// the .map() method of a DataSet object, requires the current image of the swap chain as first parameter
		// the second parameter is the pointer to the C++ data structure to transfer to the GPU
		// the third parameter is its size
		// the fourth parameter is the location inside the descriptor set of this uniform block
	}
};


// This is the main: probably you do not need to touch this!
int main() {
    MeshLoader app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}