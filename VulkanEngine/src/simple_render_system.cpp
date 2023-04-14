#include "simple_render_system.hpp"

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

    struct SimplePushConstantData {
        glm::mat2 transform{ 1.f };
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    SimpleRenderSystem::SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass)
        : m_lveDevice{ device } {
        CreatePipelineLayout();
        CreatePipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(m_lveDevice.Device(), m_pipelineLayout, nullptr);
    }

    void SimpleRenderSystem::CreatePipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(m_lveDevice.Device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void SimpleRenderSystem::CreatePipeline(VkRenderPass renderPass) {
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        LvePipeline::DefaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.m_renderPass = renderPass;
        pipelineConfig.m_pipelineLayout = m_pipelineLayout;
        m_lvePipeline = std::make_unique<LvePipeline>(
            m_lveDevice,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipelineConfig);
    }

    void SimpleRenderSystem::RenderGameObjects(
        VkCommandBuffer commandBuffer, std::vector<LveGameObject>& gameObjects) {
        m_lvePipeline->Bind(commandBuffer);

        // update
        int i = 0;
        for (auto& obj : gameObjects)
        {
            i += 1;
            obj.m_transform2D.m_rotation =
                glm::mod<float>(obj.m_transform2D.m_rotation + 0.00001f * i, 2.0f * glm::pi<float>());
        }

        for (auto& obj : gameObjects) {
            obj.m_transform2D.m_rotation = glm::mod(obj.m_transform2D.m_rotation + 0.00001f * i, glm::two_pi<float>());

            SimplePushConstantData push{};
            push.offset = obj.m_transform2D.m_translation;
            push.color = obj.m_color;
            push.transform = obj.m_transform2D.mat2();

            vkCmdPushConstants(
                commandBuffer,
                m_pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);
            obj.m_model->Bind(commandBuffer);
            obj.m_model->Draw(commandBuffer);
        }
    }

}  // namespace lve