#include "Engine.h"
#include "Instance.h"
#include "Logging.h"
#include "Device.h"
#include "SwapChain.h"
#include "Pipeline.h"
#include "Framebuffer.h"
#include "Commands.h"
#include "Sync.h"

Engine::Engine(int width, int height, GLFWwindow* window)
	: m_width{width},
	m_height{height},
	m_window{window}
{
	if (m_debugMode) 
		std::cout << "Creating the graphics engine: LearnVulkanEngine\n";

	CreateInstance();
	CreateDevice();
	CreatePipeline();
	FinalSetup();
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

	for (const vkUtil::SwapChainFrame& frame : m_swapChainFrames) 
	{
		m_device.destroyImageView(frame.imageView);
		m_device.destroyFramebuffer(frame.framebuffer);
		m_device.destroyFence(frame.inFlight);
		m_device.destroySemaphore(frame.imageAvailable);
		m_device.destroySemaphore(frame.renderFinished);
	}

	m_device.destroySwapchainKHR(m_swapChain);
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
	vkInit::SwapChainBundle bundle = vkInit::CreateSwapChain(m_device, m_physicalDevice, m_surface, m_width, m_height, m_debugMode);
	m_swapChain = bundle.swapchain;
	m_swapChainFrames = bundle.frames;
	m_swapChainFormat = bundle.format;
	m_swapChainExtent = bundle.extent;
	m_maxFramesInFlight = static_cast<int>(m_swapChainFrames.size());
	m_frameNumber = 0;
}

void Engine::CreatePipeline()
{
	vkInit::GraphicsPipelineInBundle specification = {};
	specification.device = m_device;
	specification.vertexFilepath = "shaders/vertex.spv";
	specification.fragmentFilepath = "shaders/fragment.spv";
	specification.swapchainExtent = m_swapChainExtent;
	specification.swapchainImageFormat = m_swapChainFormat;

	vkInit::GraphicsPipelineOutBundle output = vkInit::CreateGraphicsPipeline(specification, m_debugMode);

	m_pipelineLayout = output.layout;
	m_renderPass = output.renderpass;
	m_pipeline = output.pipeline;
}

void Engine::FinalSetup()
{
	vkInit::FramebufferInput frameBufferInput;
	frameBufferInput.device = m_device;
	frameBufferInput.renderPass = m_renderPass;
	frameBufferInput.swapChainExtent = m_swapChainExtent;
	vkInit::CreateFramebuffers(frameBufferInput, m_swapChainFrames, m_debugMode);

	m_commandPool = vkInit::CreateCommandPool(m_device, m_physicalDevice, m_surface, m_debugMode);

	vkInit::CommandBufferInputChunk commandBufferInput = { m_device, m_commandPool, m_swapChainFrames };
	m_mainCommandBuffer = vkInit::CreateCommandBuffers(commandBufferInput, m_debugMode);

	for (vkUtil::SwapChainFrame& frame : m_swapChainFrames) 
	{
		frame.imageAvailable = vkInit::CreateSemaphore(m_device, m_debugMode);
		frame.renderFinished = vkInit::CreateSemaphore(m_device, m_debugMode);
		frame.inFlight = vkInit::CreateFence(m_device, m_debugMode);
	}
}

void Engine::RecordDrawCommands(vk::CommandBuffer commandBuffer, uint32_t imageIndex) 
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

	vk::ClearValue clearColor{ std::array<float, 4>{1.0f, 0.5f, 0.25f, 1.0f} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
	commandBuffer.draw(3, 1, 0, 0);
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

void Engine::Render() 
{
	static_cast<void>(m_device.waitForFences(1, &(m_swapChainFrames[m_frameNumber].inFlight), VK_TRUE, UINT64_MAX));
	static_cast<void>(m_device.resetFences(1, &(m_swapChainFrames[m_frameNumber].inFlight)));

	//acquireNextImageKHR(vk::SwapChainKHR, timeout, semaphore_to_signal, fence)
	uint32_t imageIndex{ m_device.acquireNextImageKHR(m_swapChain, UINT64_MAX, m_swapChainFrames[m_frameNumber].imageAvailable, nullptr).value };

	vk::CommandBuffer commandBuffer = m_swapChainFrames[imageIndex].commandBuffer;

	commandBuffer.reset();

	RecordDrawCommands(commandBuffer, imageIndex);

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

	static_cast<void>(m_presentQueue.presentKHR(presentInfo));
	m_frameNumber = (m_frameNumber + 1) % m_maxFramesInFlight;
}