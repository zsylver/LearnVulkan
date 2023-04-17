#pragma once
#include "lve_device.hpp"

// std
#include <string>
#include <vector>

namespace lve
{
	struct PipelineConfigInfo
	{
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		std::vector<VkVertexInputBindingDescription> m_bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions{};
		VkPipelineViewportStateCreateInfo m_viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo m_rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo m_multisampleInfo;
		VkPipelineColorBlendAttachmentState m_colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo m_colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo m_depthStencilInfo;
		std::vector<VkDynamicState> m_dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo m_dynamicStateInfo;

		VkPipelineLayout m_pipelineLayout = nullptr;
		VkRenderPass m_renderPass = nullptr;
		uint32_t m_subpass = 0;
	};

	class LvePipeline
	{
	public:
		LvePipeline(
			LveDevice &device,
			const std::string& vertFilepath,
			const std::string& fragFilepath,
			const PipelineConfigInfo& configInfo);

		~LvePipeline();

		LvePipeline() = default;
		LvePipeline(const LvePipeline&) = delete;
		LvePipeline& operator=(const LvePipeline&) = delete;

		void Bind(VkCommandBuffer commandBuffer);

		static void DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

	private:
		static std::vector<char> ReadFile(const std::string& filepath);

		void CreateGraphicsPipeline(
			const std::string& vertFilepath, 
			const std::string& fragFilepath,
			const PipelineConfigInfo& configInfo);

		void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		LveDevice& m_lveDevice;
		VkPipeline m_graphicsPipeline;
		VkShaderModule m_vertShaderModule;
		VkShaderModule m_fragShaderModule;
	};
}