#pragma once

#include "Config.h"
#include "Frame.h"
#include "Scene.h"
#include "VertexManager.h"
#include "Image.h"
#include "Texture.h"
#include "CubeMap.h"

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
	std::vector<PipelineTypes> m_pipelineTypes = { { PipelineTypes::SKY, PipelineTypes::STANDARD } };
	std::unordered_map<PipelineTypes, vk::PipelineLayout> m_pipelineLayout;
	std::unordered_map<PipelineTypes, vk::RenderPass> m_renderPass;
	std::unordered_map<PipelineTypes, vk::Pipeline> m_pipeline;

	//Command-related variables
	vk::CommandPool m_commandPool;
	vk::CommandBuffer m_mainCommandBuffer;

	//Synchronization objects
	int m_maxFramesInFlight, m_frameNumber;

	//Descriptor objects
	std::unordered_map<PipelineTypes, vk::DescriptorSetLayout> m_frameSetLayout;
	vk::DescriptorPool m_frameDescriptorPool;
	std::unordered_map<PipelineTypes, vk::DescriptorSetLayout> m_meshSetLayout;
	vk::DescriptorPool m_meshDescriptorPool;

	//Asset pointers
	VertexManager* m_meshes;
	std::unordered_map<MeshTypes, vkImage::Texture*> m_materials;
	vkImage::CubeMap* m_cubeMap;

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
	void RecordDrawCommandsScene(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);
	void RecordDrawCommandsSky(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene);
	void RenderObjects(vk::CommandBuffer commandBuffer, MeshTypes objectType, uint32_t& startInstance, uint32_t instanceCount);

	void DestroySwapChain();
};