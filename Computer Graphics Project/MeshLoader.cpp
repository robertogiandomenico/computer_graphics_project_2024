// This has been adapted from the Vulkan tutorial
#define _CRT_SECURE_NO_WARNINGS

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
struct UniformBlock {
	alignas(16) glm::mat4 mvpMat;
};

// The vertices data structures
// Example
struct Vertex {
	glm::vec3 pos;
	glm::vec2 UV;
};



// MAIN ! 
class MeshLoader : public BaseProject {
	protected:

	// Current aspect ratio (used by the callback that resized the window
	float Ar;

	// Other application parameters
	glm::vec3 camPos = glm::vec3(0.0, 1.5, 7.0);
	float camYaw = 0.0f;
	float camPitch = glm::radians(-30.0f);
	float camRoll = 0.0f;
	float camDist = 9.0f;
	const glm::vec3 CamTargetDelta = glm::vec3(0, 12, 0);
	//const glm::vec3 Cam1stPos = glm::vec3(0.49061f, 2.07f, 2.7445f);
	float Yaw = 0.0f;

	// Cat initial position
	glm::vec3 catPosition = glm::vec3(0, 0, 0);
	// Cat initial orientation
	float catYaw = glm::radians(180.0f);

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSL;

	// Vertex formats
	VertexDescriptor VD;

	// Pipelines [Shader couples]
	Pipeline P;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	// Models
	Model<Vertex> M1, Mcat, Mcrystals;
	// Descriptor sets
	DescriptorSet DS1, DScat, DScrystals;
	// Textures
	Texture T1, T2;
	
	// C++ storage for uniform variables
	UniformBlock ubo1, uboCat, uboCrystals;

	// Other application parameters
	
	// Bounding boxes for the cat and the collectibles
	BoundingBox catBox = BoundingBox(catPosition, glm::vec3(0.2f, 1.0f, 1.0f));
	BoundingBox collectiblesBBs[2] = {
		BoundingBox(glm::vec3(5, 0, 5), glm::vec3(0.2f, 0.2f, 0.2f)),
		BoundingBox(glm::vec3(-4, 0, 0), glm::vec3(0.7f, 1.0f, 1.0f)),
	};

	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "Mesh Loader";
    	windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 10;
		texturesInPool = 10;
		setsInPool = 10;
		
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
				         sizeof(glm::vec2), UV}
				});

		// Pipelines [Shader couples]
		// The second parameter is the pointer to the vertex definition
		// Third and fourth parameters are respectively the vertex and fragment shaders
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		P.init(this, &VD, "shaders/ShaderVert.spv", "shaders/ShaderFrag.spv", {&DSL});

		// Models, textures and Descriptors (values assigned to the uniforms)

		// Create models
		// The second parameter is the pointer to the vertex definition for this model
		// The third parameter is the file name
		// The last is a constant specifying the file type: currently only OBJ or GLTF
		M1.init(this, &VD, "Models/Cube.obj", OBJ);

		Mcat.init(this, &VD, "Models/Cat3D.gltf", GLTF);
		Mcrystals.init(this, &VD, "Models/crystals.obj", OBJ);
		
		// Create the textures
		// The second parameter is the file name
		T1.init(this, "textures/Checker.png");
		T2.init(this, "textures/Textures.png");
		
		// Init local variables
	}
	
	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		P.create();

		// Here you define the data set
		DS1.init(this, &DSL, {
		// the second parameter, is a pointer to the Uniform Set Layout of this set
		// the last parameter is an array, with one element per binding of the set.
		// first  elmenet : the binding number
		// second element : UNIFORM or TEXTURE (an enum) depending on the type
		// third  element : only for UNIFORMs, the size of the corresponding C++ object. For texture, just put 0
		// fourth element : only for TEXTUREs, the pointer to the corresponding texture object. For uniforms, use nullptr
					{0, UNIFORM, sizeof(UniformBlock), nullptr},
					{1, TEXTURE, 0, &T1}
				});
		
		DScat.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBlock), nullptr},
					{1, TEXTURE, 0, &T2}
			});
		DScrystals.init(this, &DSL, {
					{0, UNIFORM, sizeof(UniformBlock), nullptr},
					{1, TEXTURE, 0, &T2}
			});
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		P.cleanup();

		// Cleanup datasets
		DS1.cleanup();

		DScat.cleanup();
		DScrystals.cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() {
		// Cleanup textures
		T1.cleanup();
		T2.cleanup();
		
		// Cleanup models
		M1.cleanup();
		
		Mcat.cleanup();
		Mcrystals.cleanup();
		
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
		DS1.bind(commandBuffer, P, 0, currentImage);
		// For a Dataset object, this command binds the corresponing dataset
		// to the command buffer and pipeline passed in its first and second parameters.
		// The third parameter is the number of the set being bound
		// As described in the Vulkan tutorial, a different dataset is required for each image in the swap chain.
		// This is done automatically in file Starter.hpp, however the command here needs also the index
		// of the current image in the swap chain, passed in its last parameter
					
		// binds the model
		M1.bind(commandBuffer);
		// For a Model object, this command binds the corresponing index and vertex buffer
		// to the command buffer passed in its parameter
		
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(M1.indices.size()), 1, 0, 0, 0);
		// the second parameter is the number of indexes to be drawn. For a Model object,
		// this can be retrieved with the .indices.size() method.


		DScat.bind(commandBuffer, P, 0, currentImage);
		Mcat.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(Mcat.indices.size()), 1, 0, 0, 0);

		DScrystals.bind(commandBuffer, P, 0, currentImage);
		Mcrystals.bind(commandBuffer);
		vkCmdDrawIndexed(commandBuffer,
			static_cast<uint32_t>(Mcrystals.indices.size()), 1, 0, 0, 0);
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
			BoundingBox newCatBox = BoundingBox(catPosition, glm::vec3(3.0f, 3.0f, 3.0f));
			for (int i = 0; i < 2; i++) {	// 2 = collectiblesBBs.size()
				if (newCatBox.intersects(collectiblesBBs[i])) {
					catPosition += cameraForward * m.z * MOVE_SPEED * deltaT;
					//catPosition.y -= m.y * MOVE_SPEED * deltaT;
					catPosition -= cameraRight * m.x * MOVE_SPEED * deltaT;
				}
			}
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

		// Update the uniforms for each object
		glm::mat4 World;

		// Cube object
		World = glm::translate(glm::mat4(1), glm::vec3(-4, 0, 0));
		ubo1.mvpMat = ViewPrj * World;
		DS1.map(currentImage, &ubo1, sizeof(ubo1), 0);
		// the .map() method of a DataSet object, requires the current image of the swap chain as first parameter
		// the second parameter is the pointer to the C++ data structure to transfer to the GPU
		// the third parameter is its size
		// the fourth parameter is the location inside the descriptor set of this uniform block
		
		// Cat object
		World = glm::translate(glm::mat4(1), catPosition) *
			glm::rotate(glm::mat4(1), catYaw, glm::vec3(0, 1, 0)) * // Rotate the cat to face its movement direction
			glm::scale(glm::mat4(1), glm::vec3(3.0f));
		uboCat.mvpMat = ViewPrj * World;
		DScat.map(currentImage, &uboCat, sizeof(uboCat), 0);

		// Crystals object
		World = glm::translate(glm::mat4(1), glm::vec3(3, 0, 2));
			// * glm::scale(glm::mat4(1), glm::vec3(3.0f));		//how comes that it scales also the offset from the origin?
		uboCrystals.mvpMat = ViewPrj * World;
		DScrystals.map(currentImage, &uboCrystals, sizeof(uboCrystals), 0);
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