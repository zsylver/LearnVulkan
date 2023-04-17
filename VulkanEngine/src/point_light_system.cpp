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

    struct PointLightPushConstants 
    {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

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
         VkPushConstantRange pushConstantRange{};
         pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
         pushConstantRange.offset = 0;
         pushConstantRange.size = sizeof(PointLightPushConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
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

    void PointLightSystem::Update(FrameInfo& frameInfo, GlobalUBO& ubo)
    {
        auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.m_frameTime, { 0.f, -1.f, 0.f });
        int lightIndex = 0;
        for (auto& kv : frameInfo.m_gameObjects) {
            auto& obj = kv.second;
            if (obj.m_pointLight == nullptr) continue;

            assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

            // update light position
            obj.m_transform.m_translation = glm::vec3(rotateLight * glm::vec4(obj.m_transform.m_translation, 1.f));

            // copy light to ubo
            ubo.pointLights[lightIndex].position = glm::vec4(obj.m_transform.m_translation, 1.f);
            ubo.pointLights[lightIndex].color = glm::vec4(obj.m_color, obj.m_pointLight->m_lightIntensity);

            lightIndex += 1;
        }
        ubo.numLights = lightIndex;
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

        for (auto& kv : frameInfo.m_gameObjects) 
        {
            auto& obj = kv.second;
            if (obj.m_pointLight == nullptr) continue;

            PointLightPushConstants push{};
            push.position = glm::vec4(obj.m_transform.m_translation, 1.f);
            push.color = glm::vec4(obj.m_color, obj.m_pointLight->m_lightIntensity);
            push.radius = obj.m_transform.m_scale.x;

            vkCmdPushConstants(
                frameInfo.m_commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PointLightPushConstants),
                &push);
            vkCmdDraw(frameInfo.m_commandBuffer, 6, 1, 0, 0);
        }
    }

}  // namespace lve