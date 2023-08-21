#include "Engine.h"
#include "Instance.h"
#include "Logging.h"
#include "Device.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "Framebuffer.h"
#include "Commands.h"
#include "Sync.h"
#include "Descriptors.h"
#include "ObjMesh.h"
#include "Mesh.h"
#include "Texture.h"
#include "CubeMap.h"

Engine::Engine(int width, int height, GLFWwindow* window)
	: m_width{ width },
	m_height{ height },
	m_window{ window }
{
	if (m_debugMode) 
		std::cout << "Creating the graphics engine: LearnVulkanEngine\n";

	CreateInstance();
	CreateDevice();
	CreateDescriptorSetLayout();
	CreatePipeline();
	FinalSetup();
	CreateAssets();
}

void Engine::DestroySwapChain()
{
	for (vkUtil::SwapChainFrame& frame : m_swapChainFrames)
	{
		frame.Destroy();
	}

	m_device.destroySwapchainKHR(m_swapChain);

	m_device.destroyDescriptorPool(m_frameDescriptorPool);
}

Engine::~Engine() 
{
	m_device.waitIdle();

	if (m_debugMode) 
		std::cout << "Closing the graphics engine: LearnVulkanEngine\n";

	m_device.destroyCommandPool(m_commandPool);

	for (PipelineTypes pipelineType : m_pipelineTypes)
	{
		m_device.destroyPipeline(m_pipeline[pipelineType]);
		m_device.destroyPipelineLayout(m_pipelineLayout[pipelineType]);
		m_device.destroyRenderPass(m_renderPass[pipelineType]);
	}

	DestroySwapChain();

	for (PipelineTypes pipelineType : m_pipelineTypes)
	{
		m_device.destroyDescriptorSetLayout(m_frameSetLayout[pipelineType]);
		m_device.destroyDescriptorSetLayout(m_meshSetLayout[pipelineType]);
	}
	m_device.destroyDescriptorPool(m_meshDescriptorPool);

	delete m_meshes;

	for (const auto& [key, texture] : m_materials)
	{
		delete texture;
	}

	delete m_cubeMap;

	m_device.destroy();

	m_instance.destroySurfaceKHR(m_surface);

	if (m_debugMode)
		m_instance.destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, m_dispatchLoaderInstance);
	
	m_instance.destroy();
	glfwTerminate();
}

void Engine::CreateInstance()
{
	m_instance = vkInit::CreateInstance(m_debugMode, "LearnVulkanEngine");
	m_dispatchLoaderInstance = vk::DispatchLoaderDynamic(m_instance, vkGetInstanceProcAddr);

	if (m_debugMode)
		m_debugMessenger = vkInit::CreateDebugMessenger(m_instance, m_dispatchLoaderInstance);

	VkSurfaceKHR cStyleSurface;
	if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &cStyleSurface) != VK_SUCCESS) 
	{
		if (m_debugMode)
			std::cout << "Failed to abstract glfw surface for Vulkan\n";
	}
	else if (m_debugMode)
	{
		std::cout << "Successfully abstracted glfw surface for Vulkan\n";
	}

	//copy constructor converts to hpp convention
	m_surface = cStyleSurface;
}

void Engine::CreateDevice()
{
	m_physicalDevice = vkInit::ChoosePhysicalDevice(m_instance, m_debugMode);
	m_device = vkInit::CreateLogicalDevice(m_physicalDevice, m_surface, m_debugMode);
	std::array<vk::Queue, 2> queues = vkInit::GetQueues(m_physicalDevice, m_device, m_surface, m_debugMode);
	m_graphicsQueue = queues[0];
	m_presentQueue = queues[1];
	CreateSwapChain();
	m_frameNumber = 0;
}

void Engine::CreateDescriptorSetLayout()
{
	vkInit::DescriptorSetLayoutData bindings;
	bindings.m_count = 1;

	bindings.m_indices.push_back(0);
	bindings.m_types.push_back(vk::DescriptorType::eUniformBuffer);
	bindings.m_counts.push_back(1);
	bindings.m_stages.push_back(vk::ShaderStageFlagBits::eVertex);

	m_frameSetLayout[PipelineTypes::SKY] = vkInit::CreateDescriptorSetLayout(m_device, bindings);

	bindings.m_count = 2;

	bindings.m_indices.push_back(1);
	bindings.m_types.push_back(vk::DescriptorType::eStorageBuffer);
	bindings.m_counts.push_back(1);
	bindings.m_stages.push_back(vk::ShaderStageFlagBits::eVertex);

	m_frameSetLayout[PipelineTypes::STANDARD] = vkInit::CreateDescriptorSetLayout(m_device, bindings);

	bindings.m_count = 1;

	bindings.m_indices[0] = 0;
	bindings.m_types[0] = vk::DescriptorType::eCombinedImageSampler;
	bindings.m_counts[0] = 1;
	bindings.m_stages[0] = vk::ShaderStageFlagBits::eFragment;

	m_meshSetLayout[PipelineTypes::SKY] = vkInit::CreateDescriptorSetLayout(m_device, bindings);
	m_meshSetLayout[PipelineTypes::STANDARD] = vkInit::CreateDescriptorSetLayout(m_device, bindings);
}

void Engine::CreatePipeline()
{
	vkInit::PipelineBuilder pipelineBuilder(m_device);

	//Sky
	pipelineBuilder.SetOverwriteMode(false);
	pipelineBuilder.SpecifyVertexShader("shaders/sky_vertex.spv");
	pipelineBuilder.SpecifyFragmentShader("shaders/sky_fragment.spv");
	pipelineBuilder.SpecifySwapChainExtent(m_swapChainExtent);
	pipelineBuilder.ClearDepthAttachment();
	pipelineBuilder.AddDescriptorSetLayout(m_frameSetLayout[PipelineTypes::SKY]);
	pipelineBuilder.AddDescriptorSetLayout(m_meshSetLayout[PipelineTypes::SKY]);
	pipelineBuilder.AddColorAttachment(m_swapChainFormat, 0);

	vkInit::GraphicsPipelineOutBundle output = pipelineBuilder.Build();

	m_pipelineLayout[PipelineTypes::SKY] = output.layout;
	m_renderPass[PipelineTypes::SKY] = output.renderpass;
	m_pipeline[PipelineTypes::SKY] = output.pipeline;
	pipelineBuilder.Reset();

	//Standard
	pipelineBuilder.SetOverwriteMode(true);
	pipelineBuilder.SpecifyVertexFormat(
		vkMesh::GetPosColorBindingDescription(),
		vkMesh::GetPosColorAttributeDescriptions());
	pipelineBuilder.SpecifyVertexShader("shaders/vertex.spv");
	pipelineBuilder.SpecifyFragmentShader("shaders/fragment.spv");
	pipelineBuilder.SpecifySwapChainExtent(m_swapChainExtent);
	pipelineBuilder.SpecifyDepthAttachment(m_swapChainFrames[0].depthFormat, 1);
	pipelineBuilder.AddDescriptorSetLayout(m_frameSetLayout[PipelineTypes::STANDARD]);
	pipelineBuilder.AddDescriptorSetLayout(m_meshSetLayout[PipelineTypes::STANDARD]);
	pipelineBuilder.AddColorAttachment(m_swapChainFormat, 0);

	output = pipelineBuilder.Build();

	m_pipelineLayout[PipelineTypes::STANDARD] = output.layout;
	m_renderPass[PipelineTypes::STANDARD] = output.renderpass;
	m_pipeline[PipelineTypes::STANDARD] = output.pipeline;
}

void Engine::CreateSwapChain()
{
	vkInit::SwapChainBundle bundle = vkInit::CreateSwapChain(m_device, m_physicalDevice, m_surface, m_width, m_height, m_debugMode);
	m_swapChain = bundle.swapchain;
	m_swapChainFrames = bundle.frames;
	m_swapChainFormat = bundle.format;
	m_swapChainExtent = bundle.extent;
	m_maxFramesInFlight = static_cast<int>(m_swapChainFrames.size());

	for (vkUtil::SwapChainFrame& frame : m_swapChainFrames)
	{
		frame.logicalDevice = m_device;
		frame.physicalDevice = m_physicalDevice;
		frame.width = m_swapChainExtent.width;
		frame.height = m_swapChainExtent.height;

		frame.CreateDepthResources();
	}
}

void Engine::RecreateSwapChain()
{
	m_width = 0;
	m_height = 0;
	while (m_width == 0 || m_height == 0)
	{
		glfwGetFramebufferSize(m_window, &m_width, &m_height);
		glfwWaitEvents();
	}

	m_device.waitIdle();

	DestroySwapChain();
	CreateSwapChain();
	CreateFrameBuffers();
	CreateFrameResources();

	vkInit::CommandBufferInputChunk commandBufferInput = { m_device, m_commandPool, m_swapChainFrames };
	vkInit::CreateFrameCommandBuffers(commandBufferInput, m_debugMode);
}

void Engine::CreateFrameBuffers()
{
	vkInit::FramebufferInput frameBufferInput;
	frameBufferInput.device = m_device;
	frameBufferInput.renderPass = m_renderPass;
	frameBufferInput.swapChainExtent = m_swapChainExtent;
	vkInit::CreateFramebuffers(frameBufferInput, m_swapChainFrames, m_debugMode);
}

void Engine::CreateFrameResources()
{
	vkInit::DescriptorSetLayoutData bindings;
	bindings.m_count = 2;
	bindings.m_types.push_back(vk::DescriptorType::eUniformBuffer);
	bindings.m_types.push_back(vk::DescriptorType::eStorageBuffer);
	uint32_t descriptorSetsPerFrame = 2;

	m_frameDescriptorPool = vkInit::CreateDescriptorPool(m_device, static_cast<uint32_t>(m_swapChainFrames.size() * descriptorSetsPerFrame), bindings);

	for (vkUtil::SwapChainFrame& frame : m_swapChainFrames)
	{
		frame.imageAvailable = vkInit::CreateSemaphore(m_device, m_debugMode);
		frame.renderFinished = vkInit::CreateSemaphore(m_device, m_debugMode);
		frame.inFlight = vkInit::CreateFence(m_device, m_debugMode);

		frame.CreateDescriptorResources();

		frame.descriptorSet[PipelineTypes::SKY] = vkInit::AllocateDescriptorSet(m_device, m_frameDescriptorPool, m_frameSetLayout[PipelineTypes::SKY]);
		frame.descriptorSet[PipelineTypes::STANDARD] = vkInit::AllocateDescriptorSet(m_device, m_frameDescriptorPool, m_frameSetLayout[PipelineTypes::STANDARD]);

		frame.RecordWriteOperations();
	}
}

void Engine::FinalSetup()
{
	CreateFrameBuffers();
	m_commandPool = vkInit::CreateCommandPool(m_device, m_physicalDevice, m_surface, m_debugMode);

	vkInit::CommandBufferInputChunk commandBufferInput = { m_device, m_commandPool, m_swapChainFrames };
	m_mainCommandBuffer = vkInit::CreateCommandBuffer(commandBufferInput, m_debugMode);
	vkInit::CreateFrameCommandBuffers(commandBufferInput, m_debugMode);

	CreateFrameResources();
}

void Engine::CreateAssets()
{
	m_meshes = new VertexManager();
	std::unordered_map<MeshTypes, std::vector<const char*>> modelFilenames =
	{
		{MeshTypes::GROUND, {"./models/ground.obj","./models/ground.mtl"}},
		{MeshTypes::GIRL, {"./models/girl.obj","none"}},
		{MeshTypes::SKULL, {"./models/skull.obj","./models/skull.mtl"}},
		{MeshTypes::ROOM, {"./models/viking_room.obj", "none"}}
	};

	std::unordered_map<MeshTypes, glm::mat4> preTransforms =
	{
		{MeshTypes::GROUND, glm::mat4(1.f)},
		{MeshTypes::GIRL, glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))},
		{MeshTypes::SKULL, glm::mat4(1.f)},
		{MeshTypes::ROOM, glm::rotate(glm::mat4(1.f), glm::radians(135.f), glm::vec3(0.f, 0.f, 1.f))}
	};

	for (std::pair<MeshTypes, std::vector<const char*>> pair : modelFilenames)
	{
		vkMesh::ObjMesh model(preTransforms[pair.first], pair.second[0], pair.second[1]);
		m_meshes->Consume(pair.first, model.m_vertices, model.m_indices);
	}

	FinalizationChunk finalizationChunk;
	finalizationChunk.m_logicalDevice = m_device;
	finalizationChunk.m_physicalDevice = m_physicalDevice;
	finalizationChunk.m_queue = m_graphicsQueue;
	finalizationChunk.m_commandBuffer = m_mainCommandBuffer;

	m_meshes->Finalize(finalizationChunk);

	//Materials
	std::unordered_map<MeshTypes, std::vector<const char*>> filenames
	{
		{ MeshTypes::GROUND, {"./textures/ground.jpg"} },
		{ MeshTypes::GIRL, {"./textures/none.png"} },
		{ MeshTypes::SKULL, {"./textures/skull.png"} },
		{ MeshTypes::ROOM, {"./textures/viking_room.png"} }
	};

	//Make a descriptor pool to allocate sets
	vkInit::DescriptorSetLayoutData bindings;
	bindings.m_count = 1;
	bindings.m_types.push_back(vk::DescriptorType::eCombinedImageSampler);

	m_meshDescriptorPool = vkInit::CreateDescriptorPool(m_device, static_cast<uint32_t>(filenames.size() + 1), bindings);


	vkImage::TextureInputChunk textureInfo;
	textureInfo.m_commandBuffer = m_mainCommandBuffer;
	textureInfo.m_queue = m_graphicsQueue;
	textureInfo.m_logicalDevice = m_device;
	textureInfo.m_physicalDevice = m_physicalDevice;
	textureInfo.m_layout = m_meshSetLayout[PipelineTypes::STANDARD];
	textureInfo.m_descriptorPool = m_meshDescriptorPool;

	for (const auto& [object, filename] : filenames)
	{
		textureInfo.m_filenames = filename;
		m_materials[object] = new vkImage::Texture(textureInfo);
	}

	textureInfo.m_layout = m_meshSetLayout[PipelineTypes::SKY];
	textureInfo.m_descriptorPool = m_meshDescriptorPool;
	textureInfo.m_filenames =
	{ {
		"./textures/sky_front.png",  //x+
		"./textures/sky_back.png",   //x-
		"./textures/sky_left.png",   //y+
		"./textures/sky_right.png",  //y-
		"./textures/sky_bottom.png", //z+
		"./textures/sky_top.png",    //z-
	} };

	m_cubeMap = new vkImage::CubeMap(textureInfo);
}


void Engine::PrepareScene(vk::CommandBuffer commandBuffer)
{
	vk::Buffer vertexBuffers[] = { m_meshes->m_vertexBuffer.m_buffer };
	vk::DeviceSize offsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
	commandBuffer.bindIndexBuffer(m_meshes->m_indexBuffer.m_buffer, 0, vk::IndexType::eUint32);
}

void Engine::PrepareFrame(uint32_t imageIndex, Scene* scene)
{
	vkUtil::SwapChainFrame& frame = m_swapChainFrames[imageIndex];

	glm::vec4 camVecForward = { 1.0f, 0.0f, 0.0f, 0.0f };
	glm::vec4 camVecRight = { 0.0f, -1.0f, 0.0f, 0.0f };
	glm::vec4 camVecUp = { 0.0f, 0.0f, 1.0f, 0.0f };
	frame.cameraVectorData.m_forward = camVecForward;
	frame.cameraVectorData.m_right = camVecRight;
	frame.cameraVectorData.m_up = camVecUp;
	memcpy(frame.cameraVectorWriteLocation, &(frame.cameraVectorData), sizeof(vkUtil::CameraVectors));

	// TODO: Shift to a camera class
	glm::vec3 eye = { -1.0f, 0.0f, 5.0f };
	glm::vec3 center = { 1.0f, 0.0f, 5.0f };
	glm::vec3 up = { 0.0f, 0.0f, 1.0f };
	glm::mat4 view = glm::lookAt(eye, center, up);
	glm::mat4 projection = glm::perspective(glm::radians(45.f), static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height), 0.1f, 100.f);

	projection[1][1] *= -1;

	frame.cameraMatrixData.m_view = view;
	frame.cameraMatrixData.m_projection = projection;
	frame.cameraMatrixData.m_viewProjection = projection * view;
	memcpy(frame.cameraMatrixWriteLocation, &(frame.cameraMatrixData), sizeof(vkUtil::CameraMatrices));

	size_t i = 0;
	for (std::pair<MeshTypes, std::vector<glm::vec3>> pair : scene->m_positions) 
	{
		for (glm::vec3& position : pair.second) 
		{
			frame.modelTransforms[i++] = glm::translate(glm::mat4(1.0f), position);
		}
	}

	memcpy(frame.modelBufferWriteLocation, frame.modelTransforms.data(),i * sizeof(glm::mat4));

	frame.WriteDescriptorSet();
}

void Engine::RecordDrawCommandsSky(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene)
{
	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = m_renderPass[PipelineTypes::SKY];
	renderPassInfo.framebuffer = m_swapChainFrames[imageIndex].framebuffer[PipelineTypes::SKY];
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = m_swapChainExtent;

	vk::ClearValue colorClear;
	std::array<float, 4> colors = { 1.0f, 0.5f, 0.25f, 1.0f };

	std::vector<vk::ClearValue> clearValues = { {colorClear} };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline[PipelineTypes::SKY]);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout[PipelineTypes::SKY], 0, m_swapChainFrames[imageIndex].descriptorSet[PipelineTypes::SKY], nullptr);

	m_cubeMap->Use(commandBuffer, m_pipelineLayout[PipelineTypes::SKY]);
	commandBuffer.draw(6, 1, 0, 0);

	commandBuffer.endRenderPass();
}

void Engine::RecordDrawCommandsScene(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) 
{
	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = m_renderPass[PipelineTypes::STANDARD];
	renderPassInfo.framebuffer = m_swapChainFrames[imageIndex].framebuffer[PipelineTypes::STANDARD];
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = m_swapChainExtent;

	vk::ClearValue colorClear;
	std::array<float, 4> colors = { 1.0f, 200.f / 255.f, 220.f / 255.f, 1.0f };
	colorClear.color = vk::ClearColorValue(colors);
	vk::ClearValue depthClear;

	depthClear.depthStencil = vk::ClearDepthStencilValue({ 1.0f, 0 });
	std::vector<vk::ClearValue> clearValues = { {colorClear, depthClear} };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline[PipelineTypes::STANDARD]);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout[PipelineTypes::STANDARD], 0, m_swapChainFrames[imageIndex].descriptorSet[PipelineTypes::STANDARD], nullptr);

	PrepareScene(commandBuffer);

	uint32_t startInstance = 0;

	for (std::pair<MeshTypes, std::vector<glm::vec3>> pair : scene->m_positions)
	{
		RenderObjects(commandBuffer, pair.first, startInstance, static_cast<uint32_t>(pair.second.size()));
	}

	commandBuffer.endRenderPass();
}

void Engine::RenderObjects(vk::CommandBuffer commandBuffer, MeshTypes objectType, uint32_t& startInstance, uint32_t instanceCount)
{
	int indexCount = m_meshes->m_indexCounts.find(objectType)->second;
	int firstIndex = m_meshes->m_firstIndices.find(objectType)->second;
	m_materials[objectType]->Use(commandBuffer, m_pipelineLayout[PipelineTypes::STANDARD]);
	commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex, 0, startInstance);
	startInstance += instanceCount;
}


void Engine::Render(Scene* scene)
{
	static_cast<void>(m_device.waitForFences(1, &(m_swapChainFrames[m_frameNumber].inFlight), VK_TRUE, UINT64_MAX));
	static_cast<void>(m_device.resetFences(1, &(m_swapChainFrames[m_frameNumber].inFlight)));
	//acquireNextImageKHR(vk::SwapChainKHR, timeout, semaphore_to_signal, fence)
	uint32_t imageIndex = 0;
	try 
	{
		vk::ResultValue acquire = m_device.acquireNextImageKHR(m_swapChain, UINT64_MAX, m_swapChainFrames[m_frameNumber].imageAvailable, nullptr);
		imageIndex = acquire.value;
	}
	catch (vk::OutOfDateKHRError error)
	{
		RecreateSwapChain();
		return;
	}
	catch (vk::IncompatibleDisplayKHRError error) 
	{
		RecreateSwapChain();
		return;
	}
	catch (vk::SystemError error) 
	{
		std::cout << "Failed to acquire swapchain image!" << std::endl;
	}

	vk::CommandBuffer commandBuffer = m_swapChainFrames[m_frameNumber].commandBuffer;

	commandBuffer.reset();

	PrepareFrame(imageIndex, scene);

	vk::CommandBufferBeginInfo beginInfo{};

	try
	{
		commandBuffer.begin(beginInfo);
	}
	catch (vk::SystemError err)
	{
		if (m_debugMode)
			std::cout << "Failed to begin recording command buffer!" << std::endl;
	}

	RecordDrawCommandsSky(commandBuffer, imageIndex, scene);
	RecordDrawCommandsScene(commandBuffer, imageIndex, scene);

	try
	{
		commandBuffer.end();
	}
	catch (vk::SystemError err)
	{
		if (m_debugMode)
		{
			std::cout << "Failed to record command buffer!" << std::endl;
		}
	}

	vk::SubmitInfo submitInfo{};

	vk::Semaphore waitSemaphores[]{ m_swapChainFrames[m_frameNumber].imageAvailable };
	vk::PipelineStageFlags waitStages[]{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vk::Semaphore signalSemaphores[]{ m_swapChainFrames[m_frameNumber].renderFinished };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	try 
	{
		m_graphicsQueue.submit(submitInfo, m_swapChainFrames[m_frameNumber].inFlight);
	}
	catch (vk::SystemError err) 
	{
		if (m_debugMode) 
			std::cout << "failed to submit draw command buffer!" << std::endl;
	}

	vk::PresentInfoKHR presentInfo{};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	vk::SwapchainKHR swapChains[]{ m_swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	vk::Result present;
	try
	{
		present = m_presentQueue.presentKHR(presentInfo);
	}
	catch (vk::OutOfDateKHRError error)
	{
		present = vk::Result::eErrorOutOfDateKHR;
	}

	if (present == vk::Result::eErrorOutOfDateKHR || present == vk::Result::eSuboptimalKHR)
	{
		RecreateSwapChain();
		return;
	}

	m_frameNumber = (m_frameNumber + 1) % m_maxFramesInFlight;
}