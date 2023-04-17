#include "point_light_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace lve {

    PointLightSystem::PointLightSystem(
        LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
        : m_lveDevice{ device } 
    {
        CreatePipelineLayout(globalSetLayout);
        CreatePipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem()
    {
        vkDestroyPipelineLayout(m_lveDevice.Device(), m_pipelineLayout, nullptr);
    }

    void PointLightSystem::CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout) 
    {
        // VkPushConstantRange pushConstantRange{};
        // pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        // pushConstantRange.offset = 0;
        // pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(m_lveDevice.Device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void PointLightSystem::CreatePipeline(VkRenderPass renderPass) 
    {
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        LvePipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.m_attributeDescriptions.clear();
        pipelineConfig.m_bindingDescriptions.clear();
        pipelineConfig.m_renderPass = renderPass;
        pipelineConfig.m_pipelineLayout = m_pipelineLayout;
        m_lvePipeline = std::make_unique<LvePipeline>(
            m_lveDevice,
            "shaders/point_light.vert.spv",
            "shaders/point_light.frag.spv",
            pipelineConfig);
    }

    void PointLightSystem::Render(FrameInfo& frameInfo) 
    {
        m_lvePipeline->Bind(frameInfo.m_commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.m_commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            0,
            1,
            &frameInfo.m_globalDescriptorSet,
            0,
            nullptr);

        vkCmdDraw(frameInfo.m_commandBuffer, 6, 1, 0, 0);
    }

}  // namespace lve