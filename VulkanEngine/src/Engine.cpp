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

Engine::Engine(int width, int height, GLFWwindow* window)
	: m_width{width},
	m_height{height},
	m_window{window}
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
	for (const vkUtil::SwapChainFrame& frame : m_swapChainFrames)
	{
		m_device.destroyImageView(frame.imageView);
		m_device.destroyFramebuffer(frame.framebuffer);
		m_device.destroyFence(frame.inFlight);
		m_device.destroySemaphore(frame.imageAvailable);
		m_device.destroySemaphore(frame.renderFinished);

		m_device.unmapMemory(frame.cameraDataBuffer.m_bufferMemory);
		m_device.freeMemory(frame.cameraDataBuffer.m_bufferMemory);
		m_device.destroyBuffer(frame.cameraDataBuffer.m_buffer);
	}

	m_device.destroySwapchainKHR(m_swapChain);

	m_device.destroyDescriptorPool(m_descriptorPool);
}

Engine::~Engine() 
{
	m_device.waitIdle();

	if (m_debugMode) 
		std::cout << "Closing the graphics engine: LearnVulkanEngine\n";

	m_device.destroyCommandPool(m_commandPool);

	m_device.destroyPipeline(m_pipeline);
	m_device.destroyPipelineLayout(m_pipelineLayout);
	m_device.destroyRenderPass(m_renderPass);

	DestroySwapChain();

	m_device.destroyDescriptorSetLayout(m_descriptorSetLayout);

	delete m_meshes;

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

	m_descriptorSetLayout = vkInit::CreateDescriptorSetLayout(m_device, bindings);
}

void Engine::CreatePipeline()
{
	vkInit::GraphicsPipelineInBundle specification = {};
	specification.device = m_device;
	specification.vertexFilepath = "shaders/vertex.spv";
	specification.fragmentFilepath = "shaders/fragment.spv";
	specification.swapchainExtent = m_swapChainExtent;
	specification.swapchainImageFormat = m_swapChainFormat;
	specification.descriptorSetLayout = m_descriptorSetLayout;

	vkInit::GraphicsPipelineOutBundle output = vkInit::CreateGraphicsPipeline(specification, m_debugMode);

	m_pipelineLayout = output.layout;
	m_renderPass = output.renderpass;
	m_pipeline = output.pipeline;
}

void Engine::CreateSwapChain()
{
	vkInit::SwapChainBundle bundle = vkInit::CreateSwapChain(m_device, m_physicalDevice, m_surface, m_width, m_height, m_debugMode);
	m_swapChain = bundle.swapchain;
	m_swapChainFrames = bundle.frames;
	m_swapChainFormat = bundle.format;
	m_swapChainExtent = bundle.extent;
	m_maxFramesInFlight = static_cast<int>(m_swapChainFrames.size());
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
	bindings.m_count = 1;
	bindings.m_types.push_back(vk::DescriptorType::eUniformBuffer);
	m_descriptorPool = vkInit::CreateDescriptorPool(m_device, static_cast<uint32_t>(m_swapChainFrames.size()), bindings);

	for (vkUtil::SwapChainFrame& frame : m_swapChainFrames)
	{
		frame.imageAvailable = vkInit::CreateSemaphore(m_device, m_debugMode);
		frame.renderFinished = vkInit::CreateSemaphore(m_device, m_debugMode);
		frame.inFlight = vkInit::CreateFence(m_device, m_debugMode);

		frame.CreateUBOResources(m_device, m_physicalDevice);
		frame.descriptorSet = vkInit::AllocateDescriptorSet(m_device, m_descriptorPool, m_descriptorSetLayout);
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

	std::vector<float> vertices = { {
		 0.0f, -0.05f, 0.0f, 1.0f, 0.0f,
		 0.05f, 0.05f, 0.0f, 1.0f, 0.0f,
		-0.05f, 0.05f, 0.0f, 1.0f, 0.0f
	} };
	MeshTypes type = MeshTypes::TRIANGLE;
	m_meshes->Consume(type, vertices);

	vertices = { {
		-0.05f,  0.05f, 1.0f, 0.0f, 0.0f,
		-0.05f, -0.05f, 1.0f, 0.0f, 0.0f,
		 0.05f, -0.05f, 1.0f, 0.0f, 0.0f,
		 0.05f, -0.05f, 1.0f, 0.0f, 0.0f,
		 0.05f,  0.05f, 1.0f, 0.0f, 0.0f,
		-0.05f,  0.05f, 1.0f, 0.0f, 0.0f
	} };
	type = MeshTypes::SQUARE;
	m_meshes->Consume(type, vertices);

	vertices = { {
		-0.05f, -0.025f, 0.0f, 0.0f, 1.0f,
		-0.02f, -0.025f, 0.0f, 0.0f, 1.0f,
		-0.03f,    0.0f, 0.0f, 0.0f, 1.0f,
		-0.02f, -0.025f, 0.0f, 0.0f, 1.0f,
		  0.0f,  -0.05f, 0.0f, 0.0f, 1.0f,
		 0.02f, -0.025f, 0.0f, 0.0f, 1.0f,
		-0.03f,    0.0f, 0.0f, 0.0f, 1.0f,
		-0.02f, -0.025f, 0.0f, 0.0f, 1.0f,
		 0.02f, -0.025f, 0.0f, 0.0f, 1.0f,
		 0.02f, -0.025f, 0.0f, 0.0f, 1.0f,
		 0.05f, -0.025f, 0.0f, 0.0f, 1.0f,
		 0.03f,    0.0f, 0.0f, 0.0f, 1.0f,
		-0.03f,    0.0f, 0.0f, 0.0f, 1.0f,
		 0.02f, -0.025f, 0.0f, 0.0f, 1.0f,
		 0.03f,    0.0f, 0.0f, 0.0f, 1.0f,
		 0.03f,    0.0f, 0.0f, 0.0f, 1.0f,
		 0.04f,   0.05f, 0.0f, 0.0f, 1.0f,
		  0.0f,   0.01f, 0.0f, 0.0f, 1.0f,
		-0.03f,    0.0f, 0.0f, 0.0f, 1.0f,
		 0.03f,    0.0f, 0.0f, 0.0f, 1.0f,
		  0.0f,   0.01f, 0.0f, 0.0f, 1.0f,
		-0.03f,    0.0f, 0.0f, 0.0f, 1.0f,
		  0.0f,   0.01f, 0.0f, 0.0f, 1.0f,
		-0.04f,   0.05f, 0.0f, 0.0f, 1.0f
	} };
	type = MeshTypes::STAR;
	m_meshes->Consume(type, vertices);

	FinalizationChunk finalizationChunk;
	finalizationChunk.m_logicalDevice = m_device;
	finalizationChunk.m_physicalDevice = m_physicalDevice;
	finalizationChunk.m_queue = m_graphicsQueue;
	finalizationChunk.m_commandBuffer = m_mainCommandBuffer;

	m_meshes->Finalize(finalizationChunk);
}



void Engine::PrepareScene(vk::CommandBuffer commandBuffer)
{
	vk::Buffer vertexBuffers[] = { m_meshes->m_vertexBuffer.m_buffer };
	vk::DeviceSize offsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
}

void Engine::PrepareFrame(uint32_t imageIndex)
{
	// TODO: Shift to a camera class
	glm::vec3 eye = { 1.f, 0.f, -1.f };
	glm::vec3 center = { 0.f, 0.f, 0.f };
	glm::vec3 up = { 0.f, 0.f, -1.f };
	glm::mat4 view = glm::lookAt(eye, center, up);
	glm::mat4 projection = glm::perspective(glm::radians(45.f), static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height), 0.1f, 10.f);

	projection[1][1] *= -1;

	m_swapChainFrames[imageIndex].cameraData.m_view = view;
	m_swapChainFrames[imageIndex].cameraData.m_projection = projection;
	m_swapChainFrames[imageIndex].cameraData.m_viewProjection = projection * view;
	memcpy(m_swapChainFrames[imageIndex].cameraDataWriteLocation, &(m_swapChainFrames[imageIndex].cameraData), sizeof(vkUtil::UBO));

	m_swapChainFrames[imageIndex].WriteDescriptorSet(m_device);
}

void Engine::RecordDrawCommands(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) 
{
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

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapChainFrames[imageIndex].framebuffer;
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = m_swapChainExtent;

	vk::ClearValue clearColor{ std::array<float, 4>{1.0f, 1.0f, 1.0f, 1.0f} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_swapChainFrames[imageIndex].descriptorSet, nullptr);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);

	PrepareScene(commandBuffer);

	int vertexCount = m_meshes->m_sizes.find(MeshTypes::TRIANGLE)->second;
	int firstVertex = m_meshes->m_offsets.find(MeshTypes::TRIANGLE)->second;
	for (glm::vec3 position : scene->m_trianglePositions)
	{
		glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
		vkUtil::ObjectData objectData;
		objectData.model = model;
		commandBuffer.pushConstants(m_pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(objectData), &objectData);
		commandBuffer.draw(vertexCount, 1, firstVertex, 0);
	}

	vertexCount = m_meshes->m_sizes.find(MeshTypes::SQUARE)->second;
	firstVertex = m_meshes->m_offsets.find(MeshTypes::SQUARE)->second;
	for (glm::vec3 position : scene->m_squarePositions)
	{
		glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
		vkUtil::ObjectData objectData;
		objectData.model = model;
		commandBuffer.pushConstants(m_pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(objectData), &objectData);
		commandBuffer.draw(vertexCount, 1, firstVertex, 0);
	}

	vertexCount = m_meshes->m_sizes.find(MeshTypes::STAR)->second;
	firstVertex = m_meshes->m_offsets.find(MeshTypes::STAR)->second;
	for (glm::vec3 position : scene->m_starPositions)
	{
		glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
		vkUtil::ObjectData objectData;
		objectData.model = model;
		commandBuffer.pushConstants(m_pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(objectData), &objectData);
		commandBuffer.draw(vertexCount, 1, firstVertex, 0);
	}

	commandBuffer.endRenderPass();

	try 
	{
		commandBuffer.end();
	}
	catch (vk::SystemError err) 
	{
		if (m_debugMode) 
		{
			std::cout << "failed to record command buffer!" << std::endl;
		}
	}
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

	vk::CommandBuffer commandBuffer = m_swapChainFrames[imageIndex].commandBuffer;

	commandBuffer.reset();

	PrepareFrame(imageIndex);

	RecordDrawCommands(commandBuffer, imageIndex, scene);

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