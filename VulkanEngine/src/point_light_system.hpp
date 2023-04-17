#pragma once

#include "lve_camera.hpp"
#include "lve_device.hpp"
#include "lve_frame_info.hpp"
#include "lve_game_object.hpp"
#include "lve_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace lve 
{
    class PointLightSystem 
    {
    public:
        PointLightSystem(
            LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem&) = delete;
        PointLightSystem& operator=(const PointLightSystem&) = delete;

        void Update(FrameInfo& frameInfo, GlobalUBO& ubo);
        void Render(FrameInfo& frameInfo);

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void CreatePipeline(VkRenderPass renderPass);

        LveDevice& m_lveDevice;

        std::unique_ptr<LvePipeline> m_lvePipeline;
        VkPipelineLayout m_pipelineLayout;
    };
}  // namespace lve