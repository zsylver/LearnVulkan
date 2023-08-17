#pragma once

#include "Config.h"
#include "Frame.h"
#include "Scene.h"
#include "VertexManager.h"

class Engine 
{

public:

	Engine(int width, int height, GLFWwindow* window);
	~Engine();

	void Render(Scene* scene);
private:

	//whether to print debug messages in functions
#ifdef NDEBUG
	const bool m_debugMode = false;
#else
	const bool m_debugMode = true;
#endif

	//glfw-related variables
	int m_width;
	int m_height;
	GLFWwindow* m_window{ nullptr };

	//instance-related variables
	vk::Instance m_instance{ nullptr };
	vk::DebugUtilsMessengerEXT m_debugMessenger{ nullptr };
	vk::DispatchLoaderDynamic m_dispatchLoaderInstance;
	vk::SurfaceKHR m_surface;

	//device-related variables
	vk::PhysicalDevice m_physicalDevice{ nullptr };
	vk::Device m_device{ nullptr };
	vk::Queue m_graphicsQueue{ nullptr };
	vk::Queue m_presentQueue{ nullptr };
	vk::SwapchainKHR  m_swapChain{ nullptr };
	std::vector<vkUtil::SwapChainFrame>  m_swapChainFrames;
	vk::Format m_swapChainFormat;
	vk::Extent2D m_swapChainExtent;

	//pipeline-related variables
	vk::PipelineLayout m_pipelineLayout;
	vk::RenderPass m_renderPass;
	vk::Pipeline m_pipeline;

	//Command-related variables
	vk::CommandPool m_commandPool;
	vk::CommandBuffer m_mainCommandBuffer;

	//Synchronization objects
	int m_maxFramesInFlight, m_frameNumber;

	//Descriptor objects
	vk::DescriptorSetLayout m_descriptorSetLayout;
	vk::DescriptorPool m_descriptorPool;

	//Asset pointers
	VertexManager* m_meshes;

	//Instance setup
	void CreateInstance();

	//Device setup
	void CreateDevice();
	void CreateSwapChain();
	void RecreateSwapChain();

	//Pipeline setup
	void CreateDescriptorSetLayout();
	void CreatePipeline();

	//Final setup steps
	void FinalSetup();
	void CreateFrameBuffers();
	void CreateFrameResources();

	void CreateAssets();
	void PrepareScene(vk::CommandBuffer commandBuffer);
	void PrepareFrame(uint32_t imageIndex, Scene* scene);

	void RecordDrawCommands(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);

	void DestroySwapChain();
};