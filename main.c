#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
/*#ifndef NDEBUG
#define NDEBUG
#endif*///uncoment to put out of debug mode
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#define MAX_FRAMES_IN_FLIGHT 2

typedef struct {
	uint32_t code;
	char message[256];

} errorState;
errorState erru = { 0, ""};

/*
dataType* name = NULL;
name = (dataType*)malloc(numOfEliments * sizeof(dataType));
if (name == NULL) {
	erru = (errorState){ runtimeErr, failMeasage };
	return;
}
this is a testS
*/


typedef enum {
runtimeErr = 1,
overflowErr,
invalidArgumentErr,
internalLogicErr
} errorType;

struct queueFamilyIndices {
	uint32_t graphicsFamily;
	bool graphicsFamilyExists;
	uint32_t presentFamily;
	bool presentFamilyExists;
};
const uint32_t queueFamilyTotal = 2;

struct swapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR* formats;
	VkPresentModeKHR* presentModes;
};

struct shaderCode {
	struct stat stats;
	uint32_t* code;
};

//THE LIST.
const uint32_t windowWidth = 800;
const uint32_t windowHeight = 600;
uint32_t currentFrame = 0;
GLFWwindow* window;
VkInstance instance;
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
struct queueFamilyIndices queueFamilies = {0, false, 0, false};
struct swapChainSupportDetails swapChainSupport;
uint32_t formatCount = 0;
uint32_t presentModeCount = 0;
VkDevice device;
VkQueue graphicsQueue;
VkQueue presentQueue;
VkSurfaceKHR surface;
VkSwapchainKHR swapChain;
VkImage* swapChainImages = NULL;
uint32_t swapChainImageCount = 0;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
VkImageView* swapChainImageViews = NULL;
VkRenderPass renderPass;
VkPipelineLayout pipelineLayout;
VkPipeline graphicsPipeline;
VkFramebuffer* swapChainFramebuffers;
VkCommandPool commandPool;
VkCommandBuffer commandBuffers[MAX_FRAMES_IN_FLIGHT];
VkSemaphore* imageAvailableSemaphores = NULL;
VkSemaphore* renderFinishedSemaphores = NULL; // allocated in createSyncObjects
VkFence* inFlightFences = NULL;


const char* validationLayers[] = {
	"VK_LAYER_KHRONOS_validation"
};
uint32_t requiredLayerCount = (uint32_t)(sizeof(validationLayers) / sizeof(validationLayers[0]));

const char* deviceExtensions[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
uint32_t requiredDeviceExtensionsCount = (uint32_t)(sizeof(deviceExtensions) / sizeof(deviceExtensions[0]));



#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif











void terminate() {
	//destroy all resources that have been created
	//existens of in flight fence as stand in for existence of others
	if (inFlightFences[0] != NULL) {
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, imageAvailableSemaphores[i], NULL);
			vkDestroyFence(device, inFlightFences[i], NULL);
		}
		for (unsigned int i = 0; i < swapChainImageCount; i++) {
			vkDestroySemaphore(device, renderFinishedSemaphores[i], NULL);
		}
		free(imageAvailableSemaphores);
		free(renderFinishedSemaphores);
		free(inFlightFences);
		printf("destroyed syncronization objects\n");
	}

	if (commandPool != NULL) {
		vkDestroyCommandPool(device, commandPool, NULL);
		printf("destroyed command pool\n");
	}

	if (swapChainFramebuffers != NULL) {
		for (unsigned int i = 0; i < swapChainImageCount; i++) {
			vkDestroyFramebuffer(device, swapChainFramebuffers[i], NULL);
		}
		printf("destroyed framebuffers\n");
	}

	if (graphicsPipeline != NULL) {
		vkDestroyPipeline(device, graphicsPipeline, NULL);
		printf("destroyed graphics pipeline\n");
	}

	if (pipelineLayout != NULL) {
		vkDestroyPipelineLayout(device, pipelineLayout, NULL);
		printf("destroyed pipeline layout\n");
	}

	if (renderPass != NULL) {
		vkDestroyRenderPass(device, renderPass, NULL);
		printf("destroyed render pass\n");
	}

	if (swapChainImageViews != NULL) {
		for (unsigned int i = 0; i < swapChainImageCount; i++) {
			vkDestroyImageView(device, swapChainImageViews[i], NULL);
		}
		printf("destroyed image veiws\n");
	}

	if (swapChain != NULL) {
		vkDestroySwapchainKHR(device, swapChain, NULL);
		printf("destroyed swapchain\n");
	}

	if (device != NULL) {
		vkDestroyDevice(device, NULL);
		printf("destroyed virtual device\n");
	}

	if (surface != NULL) {
		vkDestroySurfaceKHR(instance, surface, NULL);
		printf("destroyed vulkan surface\n");
	}

	if (instance != NULL) {
		vkDestroyInstance(instance, NULL);
		printf("destroyed vulkan instance\n");
	}

	if (window != NULL) {
		glfwDestroyWindow(window);
		printf("destroyed GLFW window\n");
	}

	glfwTerminate();
	printf("terminated GLFW\n");
	free(swapChainSupport.formats);
	free(swapChainSupport.presentModes);

	//print any error codes
	if (erru.code != 0) {
		printf("\nexited with error code %d, message: %s\n", erru.code, erru.message);
		exit(-1);
	}
	else {
		printf("\nexiting program with no errors\n");
		exit(0);
	}

	
}

void check() {
	if (erru.code != 0) {
		terminate();
	}
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, 1);
	}
}

void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	VkCommandBufferBeginInfo beginInfo = { 0 };
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = NULL;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		erru = (errorState){ runtimeErr, "error in recordCommandBuffer, vkBeginCommandBuffer() failed." };
		return;
	}

	VkRenderPassBeginInfo renderPassInfo = { 0 };
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = swapChainExtent;

	// i wont be using this
	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	VkViewport viewport = { 0 };
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)(swapChainExtent.width);
	viewport.height = (float)(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = { 0 };
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0); // so fucking satisfying.

	vkCmdEndRenderPass(commandBuffer);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		erru = (errorState){ runtimeErr, "error in recordCommandBuffer, failed to record command buffer" };
		return;
	}
}

void drawFrame() {
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(commandBuffers[currentFrame], 0);
	recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

	VkSubmitInfo submitInfo = {0};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[imageIndex] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		erru = (errorState){ runtimeErr, "error in drawFrame(), failed to submit draw command buffer." };
		return;
	}
	

	VkPresentInfoKHR presentInfo = {0};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = NULL;
	vkQueuePresentKHR(presentQueue, &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void mainLoop() {
	//show the previously hidden window.
	glfwShowWindow(window);

	while(!glfwWindowShouldClose(window)) {
		//process the inputs from the previous frame
		processInput(window);

		//record key presses made in the frame
		glfwPollEvents();
		drawFrame();
		check();
	}
	vkDeviceWaitIdle(device);
	
}



void createSyncObjects() {
		imageAvailableSemaphores = (VkSemaphore*)malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkSemaphore));
	if (imageAvailableSemaphores == NULL) {
		erru = (errorState){ runtimeErr, "error in createSyncObjects(), could not allocate memory for imageAvailableSemaphores" };
		return;
	}

	renderFinishedSemaphores = (VkSemaphore*)malloc(swapChainImageCount * sizeof(VkSemaphore));
	if (renderFinishedSemaphores == NULL) {
		erru = (errorState){ runtimeErr, "error in createSyncObjects(), could not allocate memory for renderFinishedSemaphores"};
		return;
	}

	inFlightFences = (VkFence*)malloc(MAX_FRAMES_IN_FLIGHT * sizeof(VkFence));
	if (inFlightFences == NULL) {
		erru = (errorState){ runtimeErr, "error in createSyncObjects(), could not allocate memory for renderFinishedSemaphores" };
		return;
	}
	VkSemaphoreCreateInfo semaphoreInfo = {0};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {0};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &imageAvailableSemaphores[i]) != VK_SUCCESS
			|| vkCreateFence(device, &fenceInfo, NULL, &inFlightFences[i]) != VK_SUCCESS)
		{
			erru = (errorState){ runtimeErr, "error in createSyncObjects(), could not create objects" };
			return;
		}
	}
	for (unsigned int i = 0; i < swapChainImageCount; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &renderFinishedSemaphores[i]) != VK_SUCCESS) {
			erru = (errorState){ runtimeErr, "error in createSyncObjects(), could not create objects" };
			return;
		}
	}
}

void createCommandBuffers() {
	VkCommandBufferAllocateInfo allocInfo = {0};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffers[i]) != VK_SUCCESS) {
			erru = (errorState){ runtimeErr, "error in createCommandBuffer(), could not allocate command buffer" };
			return;
		}
	}
}

void createCommandPool() {
	VkCommandPoolCreateInfo poolInfo = {0};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilies.graphicsFamily;
	if (vkCreateCommandPool(device, &poolInfo, NULL, &commandPool) != VK_SUCCESS) {
		erru = (errorState){ runtimeErr, "error in createCommandPool(), could not create command pool" };
		return;
	}
}

void createFramebuffers() {
	swapChainFramebuffers = (VkFramebuffer*)malloc(swapChainImageCount * sizeof(VkFramebuffer)); // swapChainImageCount == #of swapchain image veiws(swapChainImageViews.size())
	if (swapChainFramebuffers == NULL) {
		erru = (errorState){ runtimeErr, "error in createFramebuffers(), could not alocate memory for swapChainFramebuffers." };
		return;
	}
	for (unsigned int i = 0; i < swapChainImageCount; i++) {
		VkImageView attachments[] = { swapChainImageViews[i] }; // can optionally have multiple framebuffers per image veiw.

		VkFramebufferCreateInfo framebufferInfo = {0};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, NULL, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			erru = (errorState) { runtimeErr, "error in createFramebuffers(), failed to create frambuffer" };
			free(swapChainFramebuffers);
			return;
		}



	}
	//free(swapChainFramebuffers); i would just like to leave this here as a testiment to my stupidity.
}

VkShaderModule createShaderModule(const struct shaderCode code) {
	VkShaderModuleCreateInfo createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.stats.st_size;
	createInfo.pCode = code.code; //thats so fucking stupid
	VkShaderModule shaderModule;
	if ((vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS)) {
		erru = (errorState){runtimeErr, "error in createShaderModule(), failed to create shader module."};
		return (VkShaderModule){ NULL };
	}
	free(code.code); //so godamn stupid
	return shaderModule;
}

void readFile(const char* fileName, struct shaderCode* location) {
	//get the stats of the SPIR-V file
	struct stat stats;
	if (stat(fileName, &stats) != 0) {
		erru = (errorState){ runtimeErr, "error in readFile(), could not get statistics of file" };
		return;
	}
	//SPIR-V file have 4 bit words, so the total should be a multiple of 4
	if (stats.st_size % 4 != 0) {
		erru = (errorState){ runtimeErr, "error in readFile(), invalide file size" };
		return;
	}
	//open the file, and set the "filePointer" as the pointer
	FILE* filePointer = NULL;
	if (fopen_s(&filePointer, fileName, "rb") != 0) {
		erru = (errorState){ runtimeErr, "error in readFile(), failed to open file" };
		return;
	}
	//allocate memory for the file contents
	(*location).code = (uint32_t*)malloc(stats.st_size);
	if ((*location).code == NULL) {
		erru = (errorState){ runtimeErr, "error in readFile(), failed to alocate memory for file contents" };
		fclose(filePointer);
		return;
	}
	//store the file contents
	if (fread((*location).code, sizeof(uint32_t), stats.st_size / sizeof(uint32_t), filePointer) != stats.st_size / sizeof(uint32_t)) {
		erru = (errorState){ runtimeErr, "error in readFile(), could not read memory to target location" };
		free((*location).code);
		(*location).code = NULL;
		fclose(filePointer);
		return;
	}
	(*location).stats = stats;
}

void createGraphicsPipeline() {
	struct shaderCode vertShaderCode = {0};
	readFile("vert.spv", &vertShaderCode);
	check();
	if (vertShaderCode.code == NULL) {
		erru = (errorState){ runtimeErr, "error in createGraphicsPipeline(), you should never realy see this but it'l make the warning go away" };
		return;
	}


	struct shaderCode fragShaderCode = { 0 };
	readFile("frag.spv", &fragShaderCode);
	check();
	if (fragShaderCode.code == NULL) {
		erru = (errorState){ runtimeErr, "error in createGraphicsPipeline(), you should never realy see this but it'l make the warning go away" };
		return;
	}

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	check();
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
	check();

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = NULL;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = NULL;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {0};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {0};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent = swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {0};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {0};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {0};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = NULL;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {0};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState = {0};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = (uint32_t)(sizeof(dynamicStates) / sizeof(dynamicStates[0]));
	dynamicState.pDynamicStates = dynamicStates;


	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = NULL;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = NULL;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout) != VK_SUCCESS) {
		erru = (errorState){ runtimeErr, "error in createGraphicsPipeline(), failed to create pipeline layout" };
		return;
		vkDestroyShaderModule(device, fragShaderModule, NULL);
		vkDestroyShaderModule(device, vertShaderModule, NULL);
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {0};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = NULL;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline) != VK_SUCCESS) {
		erru = (errorState){ runtimeErr, "error in createGraphicsPipeline(), could not create the graphics pipeline" };
		vkDestroyShaderModule(device, fragShaderModule, NULL);
		vkDestroyShaderModule(device, vertShaderModule, NULL);
		return;
	}

	vkDestroyShaderModule(device, fragShaderModule, NULL);
	vkDestroyShaderModule(device, vertShaderModule, NULL);
}

void createRenderPass() {
	VkAttachmentDescription colorAttachment = {0};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {0};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency = {0};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {0};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, NULL, &renderPass) != VK_SUCCESS) {
		erru = (errorState){ runtimeErr, "error in createRenderPass(), could not create the render pass." };
		return;
	}
}

void createImageViews() {
	swapChainImageViews = (VkImageView*)malloc(swapChainImageCount * sizeof(VkImageView));
	if (swapChainImageViews == NULL) {
		erru = (errorState){ runtimeErr, "error in createImageViews(), could not alocate memory for swapChainImageViews"};
		return;
	}

	for (unsigned int i = 0; i < swapChainImageCount; i++) {
		VkImageViewCreateInfo createInfo = { 0 };
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; //i love how thats the official name.
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, NULL, &swapChainImageViews[i]) != VK_SUCCESS) {
			erru = (errorState){ runtimeErr, "error in createImageViews(), could not create an image veiw." };
			return;
		}
	}
}

VkExtent2D chooseSwapExtent(/*swapChainSupport.capabilities*/) {
	if (swapChainSupport.capabilities.currentExtent.width != UINT32_MAX) {
		return swapChainSupport.capabilities.currentExtent;
	}
	else {
		int width;
		int height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			(uint32_t)width,
			(uint32_t)height
		};

		if (actualExtent.height < swapChainSupport.capabilities.minImageExtent.height) {
			height = swapChainSupport.capabilities.minImageExtent.height;
		}
		else if (actualExtent.height > swapChainSupport.capabilities.maxImageExtent.height) {
			height = swapChainSupport.capabilities.maxImageExtent.height;
		}
		//else keep the same
		if (actualExtent.height < swapChainSupport.capabilities.minImageExtent.height) {
			height = swapChainSupport.capabilities.minImageExtent.height;
		}
		else if (actualExtent.height > swapChainSupport.capabilities.maxImageExtent.height) {
			height = swapChainSupport.capabilities.maxImageExtent.height;
		}
		//else keep the same

		return actualExtent;
	}
}

VkPresentModeKHR chooseSwapPresentMode(/*swapChainSupport.presentModes*/) {
	for (unsigned int i = 0; i < presentModeCount; i++) {
		if (swapChainSupport.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return swapChainSupport.presentModes[i];
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(/*swapChainSupport.formats*/) {
	for (unsigned int i = 0; i < formatCount; i++) {
		if (swapChainSupport.formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && swapChainSupport.formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return(swapChainSupport.formats[i]);
		}
	}

	return swapChainSupport.formats[0];
}

void createSwapChain() {
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat();
	VkPresentModeKHR presentMode = chooseSwapPresentMode();
	VkExtent2D extent = chooseSwapExtent();

	uint32_t imageCount = 3;
	if (imageCount < swapChainSupport.capabilities.minImageCount) {
		imageCount = swapChainSupport.capabilities.minImageCount;
	}
	else if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = { queueFamilies.graphicsFamily, queueFamilies.presentFamily };
	if (queueFamilies.graphicsFamily != queueFamilies.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = NULL;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;


	if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain) != VK_SUCCESS) {
		erru = (errorState){ runtimeErr, "error in createSwapChain(), failed to create swapchain" };
		return;
	}
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, NULL);

	swapChainImages = (VkImage*)malloc(imageCount * sizeof(VkImage));
	if (swapChainImages == NULL) {
		erru = (errorState){ runtimeErr, "error in createSwapChain(), could not alocate memory to store swapChainImages"};
		return;
	}
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages);

	swapChainImageCount = imageCount;
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void createLogicalDevice() {
	
	uint32_t uniqueFamilys[2]; //queueFamilyTotal
	int uniqueCount = 0;

	uniqueFamilys[0] = queueFamilies.graphicsFamily;
	uniqueCount++;

	if (queueFamilies.presentFamily != uniqueFamilys[0]) {
		printf("present family detected as distinct from graphics family.\n");
		uniqueFamilys[1] = queueFamilies.presentFamily;
		uniqueCount++;
	}

	VkDeviceQueueCreateInfo* queueCreateInfos = NULL;
	queueCreateInfos = (VkDeviceQueueCreateInfo*)malloc(uniqueCount * sizeof(VkDeviceQueueCreateInfo));
	if (queueCreateInfos == NULL) {
		erru = (errorState){ runtimeErr, "error in createLogicalDevice(), could not alocate memory for queueCreateInfos."};
		return; 
	}


	float queuePriority = 1.0f;
	for (int i = 0; i < uniqueCount; i++) {
		VkDeviceQueueCreateInfo queueCreateInfo = { 0 };
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = uniqueFamilys[i];
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos[i] = queueCreateInfo;
	}



	VkPhysicalDeviceFeatures deviceFeatures = {0};

	VkDeviceCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = uniqueCount;
	createInfo.pQueueCreateInfos = queueCreateInfos;
	
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = requiredDeviceExtensionsCount;
	createInfo.ppEnabledExtensionNames = deviceExtensions;

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = requiredLayerCount;
		createInfo.ppEnabledLayerNames = validationLayers;
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	

	if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device) != VK_SUCCESS) {
		erru = (errorState) { runtimeErr, "error in createLogicalDevice(), failed to create logical device" };
		return;
	}
	vkGetDeviceQueue(device, queueFamilies.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueFamilies.presentFamily, 0, &presentQueue);

}

void querySwapChainSupport(VkPhysicalDevice device) {
	
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainSupport.capabilities);

	uint32_t localFormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &localFormatCount, NULL);

	if (localFormatCount != 0) {

		swapChainSupport.formats = (VkSurfaceFormatKHR*)malloc(localFormatCount * sizeof(VkSurfaceFormatKHR));
		if (swapChainSupport.formats == NULL) {
			erru = (errorState){ runtimeErr, "error in 'querySwapChainSupport' could not allocate memory for details.formats" };
			return;
		}
		formatCount = localFormatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &localFormatCount, swapChainSupport.formats);
	}

	uint32_t localPresentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &localPresentModeCount, NULL);

	if (localPresentModeCount != 0) {

		swapChainSupport.presentModes = (VkPresentModeKHR*)malloc(localPresentModeCount * sizeof(VkPresentModeKHR));
		if (swapChainSupport.presentModes == NULL) {
			erru = (errorState){ runtimeErr, "error in 'querySwapChainSupport' could not allocate memory for details.formats" };
			return;
		}
		presentModeCount = localPresentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &localPresentModeCount, swapChainSupport.presentModes);
	}
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

	VkExtensionProperties* availableExtensions = NULL;
	availableExtensions = (VkExtensionProperties*)malloc(extensionCount * sizeof(VkExtensionProperties));
	if (availableExtensions == NULL) {
		erru = (errorState){ runtimeErr, "error in checkDeviceExtensionSupport(), could not alocate memory for device extensions" }; 
		terminate();
		return false;
	}
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

	//for all required device extensions
	for (unsigned int i = 0; i < requiredDeviceExtensionsCount; i++) {

		bool deviceExtencionFound = false;
		//check if there included in the available extencions
		for (unsigned int l = 0; l < extensionCount; l++) {
			if (strcmp(deviceExtensions[i], availableExtensions[l].extensionName) == 0) {
				deviceExtencionFound = true;
				break;
			}
		}
		//if not then return an error
		if (!deviceExtencionFound) {
			free(availableExtensions);
			return false;

		}
	}

	free(availableExtensions);
	return true;
}
//function paramiters of the current device, and output of where you want to store the queue indicies.
void findQueueFamilies(VkPhysicalDevice device, struct queueFamilyIndices* indices) {
	//get all queue familys
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
	VkQueueFamilyProperties* queueFamilies = NULL;
	queueFamilies = (VkQueueFamilyProperties*)malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
	if (queueFamilies == NULL) {
		erru = (errorState){ runtimeErr, "error in 'findQueueFamilies()', could not allocate memory to store available queue families" };
		return;
	}
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

	//for all available families, check if they support a graphics and present queue, then store the index of that family,
	//making sure the first one available is the one thats used
	VkBool32 presentSupport = false;
	for (unsigned int i = 0; i < queueFamilyCount; i++) {
		//if the family being checked supports a graphics Family
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			(*indices).graphicsFamily = i;
			(*indices).graphicsFamilyExists = true;
		}
		//same thing as above, only VK_QUEUE_PRESENT_BIT bit doesent exist?
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport) {
			(*indices).presentFamily = i;
			(*indices).presentFamilyExists = true;
		}
		//will favor graphics and present families being the same.
		if ((*indices).presentFamilyExists && (*indices).graphicsFamilyExists) {
			break;
		}
	}
	//custom error handling system, probobly redundant here.
	if (!((*indices).presentFamilyExists && (*indices).graphicsFamilyExists)) {
		erru = (errorState){ runtimeErr, "error in findQueueFamilies(), could not find either graphics or present Families" };
		free(queueFamilies);
		return;
	}


	free(queueFamilies);
}

bool isDeviceSuitable(VkPhysicalDevice device) {
	//get the location of all the required queue familys
	findQueueFamilies(device, &queueFamilies);
	check();

	//check if the gpu is'nt an intigrated gpu, and that it supports geometry shader
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		querySwapChainSupport(device);
		check();
		swapChainAdequate = (swapChainSupport.formats != NULL) && (swapChainSupport.presentModes != NULL);
	}

	return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			&& deviceFeatures.geometryShader 
			&& queueFamilies.graphicsFamilyExists
			&& queueFamilies.presentFamilyExists
			&& extensionsSupported
			&& swapChainAdequate;
}

void pickPhysicalDevice() {
	//find number of available GPUs
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
	if (deviceCount == 0) {
		erru = (errorState){ runtimeErr, "failed to find gpu's with vulcan support!" };
		return;
	}
	//get all available GPUs
	VkPhysicalDevice* devices = NULL;
	devices = (VkPhysicalDevice*)malloc(deviceCount * sizeof(VkPhysicalDevice));
	if (devices == NULL) {
		erru = (errorState){ runtimeErr,"failed to alocate momory to get available GPUs." };
		return;
	}
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

	//get all suitible GPUs from available ones
	int suitableDevicesTotal = 0;
	VkPhysicalDevice* suitableDevices = NULL;
	suitableDevices = (VkPhysicalDevice*)malloc(deviceCount * sizeof(VkPhysicalDevice));
	if (suitableDevices == NULL) {
		erru = (errorState){ runtimeErr,"failed to alocate momory to get available GPUs." };
		return;
	}
	for (unsigned int i = 0; i < deviceCount; i++) {
		if (isDeviceSuitable(devices[i])) {
			suitableDevices[suitableDevicesTotal] = devices[i];
			suitableDevicesTotal++;
		}
	}

	//handles no suitable GPUs, 1 GPU, and multiple GPUs
	if (suitableDevicesTotal == 0) {
		erru = (errorState){ runtimeErr, "failed to find a suitable GPU." };
		free(devices);
		free(suitableDevices);
		return;
	}
	else if (suitableDevicesTotal == 1) {
		physicalDevice = suitableDevices[0];
	}
	else {
		//store user choice(default to first)
		unsigned int userChoice = 0;
		//store the propertys of GPUs (unavailable in suitableDevices struct)
		VkPhysicalDeviceProperties deviceProperties;
		//print all the options
		printf("\n\nmultiple suitable GPUs detected, please enter the number of the one you want to use.\n");
		for (int i = 0; i < suitableDevicesTotal; i++) {
			vkGetPhysicalDeviceProperties(suitableDevices[i], &deviceProperties);
			printf("%d) %s\n", i, deviceProperties.deviceName);
		}
		//get user choice(validating that it's a valid choice)
		while (true) {
			scanf_s("%u", &userChoice);

			if (userChoice < (unsigned int)suitableDevicesTotal) {
				physicalDevice = suitableDevices[userChoice];
				break;
			}
			else {
				printf("please select valid GPU.\n");
			}
		}

	}

	free(devices);
	free(suitableDevices);
	
	
}

void createSurface() {
	if (glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS) {
		erru = (errorState){ runtimeErr, "failed to create vulkan surface." };
		return;
	}
}

void checkValidationLayerSupport() {
	//get all the available validation layers
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);
	VkLayerProperties* availableLayers = NULL;
	availableLayers = (VkLayerProperties*)malloc(layerCount * sizeof(VkLayerProperties));
	if (availableLayers == NULL) {
		erru = (errorState){ runtimeErr, "error in 'checkValidationLayerSupport()', could not allocate memory to store available layers." };
		return;
	}
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

	//for all the required validation layers
	for (unsigned int i = 0; i < requiredLayerCount; i++) {
		bool layerFound = false;
		//check if there included in the available layers
		for (unsigned int l = 0; l < layerCount; l++) {
			if (strcmp(validationLayers[i], availableLayers[l].layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		//if not then return an error
		if (!layerFound) {
			free(availableLayers);
			erru = (errorState){ runtimeErr, "could not find validation layer." };
			return;

		}
	}

	free(availableLayers);
}

void createInstance() {
	
	if (enableValidationLayers) {
		checkValidationLayerSupport();
		check();
	}

	//varyous input settings for vulkan, mostly about the type of aplication.
	VkApplicationInfo appInfo = { 0 };
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	//get required extencions, 
	uint32_t requiredExtensionCount = 0;
	const char** requiredExtensions;
	requiredExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);
	createInfo.enabledExtensionCount = requiredExtensionCount;
	createInfo.ppEnabledExtensionNames = requiredExtensions;
	//report needed validation layers
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = requiredLayerCount;
		createInfo.ppEnabledLayerNames = validationLayers;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = NULL;
	}
	
	//check if required extencions are included in available extensions
	//get available extencions
	uint32_t availableExtensionCount;
	vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, NULL);
	VkExtensionProperties* availableExtensions = NULL;
	availableExtensions = (VkExtensionProperties*)malloc(availableExtensionCount * sizeof(VkExtensionProperties));
	if (availableExtensions == NULL) {
		erru = (errorState){ runtimeErr, "error in 'createInstance()', could not allocate memory to store available extencions" };
		return;
	}
	vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, availableExtensions);

	//for all the required extencions
	for (unsigned int i = 0; i < requiredExtensionCount; i++) {
		bool extensionFound = false;
		//check if there included in the available extensions
		for (unsigned int l = 0; l < availableExtensionCount; l++) {
			if (strcmp(requiredExtensions[i], availableExtensions[l].extensionName) == 0) {
				extensionFound = true;
				break;
			}
		}
		//if not then return an error
		if (!extensionFound) {
			free(availableExtensions);
			erru = (errorState){ runtimeErr, "could not find required extension." };
			return;
		}
	}
	
	//assuming evrything went smothly, free extensions, then check if instance creation was a success 
	free(availableExtensions);
	if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
		erru = (errorState){ runtimeErr, "failed to create vk instance" };
		return;
	}

	
}

void initiateVulcan() {
	createInstance();
	check();
	createSurface();
	check();
	pickPhysicalDevice();
	check();
	createLogicalDevice();
	check();
	createSwapChain();
	check();
	createImageViews();
	check();
	createRenderPass();
	check();
	createGraphicsPipeline();
	check();
	createFramebuffers();
	check();
	createCommandPool();
	check();
	createCommandBuffers();
	check();
	createSyncObjects();
	check();
}

void initWindow() {
	

	if (glfwInit() == GLFW_FALSE) {
		erru = (errorState){ runtimeErr, "failed initiate glfw in 'initWindow'" };
		return;
	};

	//not using openGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//don't make window resizable
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	//make window invisible until the main loop
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	window = glfwCreateWindow(windowWidth, windowHeight, "Vulkan", NULL, NULL);

	if (window == NULL) {
		erru = (errorState){ runtimeErr, "failed to create window in 'initWindow'" };
		return;
	}

	
}



void app() {
	initWindow();
	check();
	initiateVulcan();
	check();
	mainLoop();
	check();
	terminate();
}

int main() {
	app();
	printf("reached end of instructions without propper termination path");
	return -1;
}

