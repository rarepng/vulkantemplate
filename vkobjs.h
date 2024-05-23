#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>
#include <vk/VkBootstrap.h>
#include <vk/vk_mem_alloc.h>

struct vkvert {
	glm::vec3 pos;
	glm::vec2 uv;
};
struct vkmesh {
	std::vector<vkvert> verts;
};
struct vkobjs {
	VmaAllocator rdallocator;

	vkb::Instance rdvkbinstance{};
	vkb::Device rdvkbdevice{};
	vkb::Swapchain rdvkbswapchain{};

	std::vector<VkImage> rdswapchainimages;
	std::vector<VkImageView> rdswapchainimageviews;
	std::vector<VkFramebuffer> rdframebuffers;

	VkQueue rdgraphicsqueue = VK_NULL_HANDLE;
	VkQueue rdpresentqueue = VK_NULL_HANDLE;

	VkImage rddepthimage = VK_NULL_HANDLE;
	VkImageView rddepthimageview = VK_NULL_HANDLE;
	VkFormat rddepthformat;
	VmaAllocation rddepthimagealloc = VK_NULL_HANDLE;

	VkRenderPass rdrenderpass = VK_NULL_HANDLE;
	VkPipelineLayout rdpipelinelayout = VK_NULL_HANDLE;
	VkPipeline rdpipeline = VK_NULL_HANDLE;

	VkCommandPool rdcommandpool = VK_NULL_HANDLE;
	VkCommandBuffer rdcommandbuffer = VK_NULL_HANDLE;

	VkSemaphore rdpresentsemaphore = VK_NULL_HANDLE;
	VkSemaphore rdrendersemaphore = VK_NULL_HANDLE;
	VkFence rdrenderfence = VK_NULL_HANDLE;

	VkImage rdtextureimage = VK_NULL_HANDLE;
	VkImageView rdtextureimageview = VK_NULL_HANDLE;
	VkSampler rdtexturesampler = VK_NULL_HANDLE;
	VmaAllocation rdtextureimagealloc = VK_NULL_HANDLE;

	VkDescriptorPool rddescriptorpool = VK_NULL_HANDLE;
	VkDescriptorSetLayout rdtexturelayout = VK_NULL_HANDLE;
	VkDescriptorSet rddescriptorset = VK_NULL_HANDLE;
};















