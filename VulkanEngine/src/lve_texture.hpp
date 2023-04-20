#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "lve_buffer.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <stdexcept>

namespace lve
{
    class LveTexture
    {
    public:
        LveTexture();
        ~LveTexture()
        {

        }

        LveTexture(const LveTexture&) = delete;
        LveTexture& operator=(const LveTexture&) = delete;

        void CreateImage(
            uint32_t width,
            uint32_t height,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties) 
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = width;
            imageInfo.extent.height = height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = format;
            imageInfo.tiling = tiling;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = usage;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateImage(m_lveDevice.Device(), &imageInfo, nullptr, &m_textureImage) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image!");
            }

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(m_lveDevice.Device(), m_textureImage, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = m_lveDevice.FindMemoryType(memRequirements.memoryTypeBits, properties);

            if (vkAllocateMemory(m_lveDevice.Device(), &allocInfo, nullptr, &m_textureImageMemory) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate image memory!");
            }

            vkBindImageMemory(m_lveDevice.Device(), m_textureImage, m_textureImageMemory, 0);
        }

        void CreateTextureImage()
        {
            int texWidth, texHeight, texChannels;
            stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            
            // 4 bytes per pixel hence multiply by 4
            VkDeviceSize imageSize = texWidth * texHeight * 4;

            if (!pixels)
            {
                throw std::runtime_error("failed to load texture image!");
            }

            //LveBuffer stagingBuffer
            //{
            //     m_lveDevice,
            //     imageSize,
            //     m_vertexCount,
            //     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            //};

            //stagingBuffer.Map();
            //stagingBuffer.WriteToBuffer((void*)vertices.data());

            //m_vertexBuffer = std::make_unique<LveBuffer>(
            //    m_lveDevice,
            //    vertexSize,
            //    m_vertexCount,
            //    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            //    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            //m_lveDevice.CopyBuffer(stagingBuffer.GetBuffer(), m_vertexBuffer->GetBuffer(), bufferSize);
        
            stbi_image_free(pixels);

            CreateImage
            (
                texWidth,
                texHeight,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );
        }

    private:
        LveDevice m_lveDevice;

        VkImage m_textureImage;
        VkDeviceMemory m_textureImageMemory;
    };
}