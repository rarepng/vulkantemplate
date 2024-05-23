#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <cstring>
#include "commandbuffer.h"
#include "texture.h"
#include "logger.h"

bool texture::loadtexture(vkobjs& rdata, std::string filename) {
	int w;
	int h;
	int c;

	unsigned char* data = stbi_load(filename.c_str(), &w, &h, &c, STBI_rgb_alpha);
	if (!data) {
		logger::log(1, "%s error: could not load file '%s'\n", __FUNCTION__, filename.c_str());
		stbi_image_free(data);
		return false;
	}


	VkDeviceSize imgsize = w * h * 4;

	VkImageCreateInfo imginfo{};
	imginfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imginfo.imageType = VK_IMAGE_TYPE_2D;
	imginfo.extent.width = static_cast<uint32_t>(w);
	imginfo.extent.height = static_cast<uint32_t>(h);
	imginfo.extent.depth = 1;
	imginfo.mipLevels = 1;
	imginfo.arrayLayers = 1;
	imginfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imginfo.tiling = VK_IMAGE_TILING_LINEAR;
	imginfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imginfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imginfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imginfo.samples = VK_SAMPLE_COUNT_1_BIT;





	VmaAllocationCreateInfo iainfo{};
	iainfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
	if (vmaCreateImage(rdata.rdallocator, &imginfo, &iainfo, &rdata.rdtextureimage, &rdata.rdtextureimagealloc, nullptr) != VK_SUCCESS) {
		logger::log(1, "%s error: could not allocate texture image via VMA\n", __FUNCTION__);
		return false;
	}





	VkBufferCreateInfo stagingbufferinfo{};
	stagingbufferinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	stagingbufferinfo.size = imgsize;
	stagingbufferinfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VkBuffer stagingbuffer;
	VmaAllocation stagingbufferalloc;

	VmaAllocationCreateInfo stagingallocinfo{};
	stagingallocinfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;


	if (vmaCreateBuffer(rdata.rdallocator,&stagingbufferinfo,&stagingallocinfo,&stagingbuffer,&stagingbufferalloc,nullptr) != VK_SUCCESS) {
		logger::log(1, "%s error: could not allocate texture staging buffer via VMA\n", __FUNCTION__);
		return false;
	}


	void* tmp;
	vmaMapMemory(rdata.rdallocator, stagingbufferalloc, &tmp);
	std::memcpy(tmp, data, static_cast<uint32_t>(imgsize));
	vmaUnmapMemory(rdata.rdallocator, stagingbufferalloc);

	stbi_image_free(data);


	VkCommandBuffer stagingcommandbuffer;
	if (!commandbuffer::init(rdata, stagingcommandbuffer)) {
		logger::log(1, "%s error: could not create texture upload command buffers\n", __FUNCTION__);
		return false;
	}

	VkImageSubresourceRange stagingbufferrange{};
	stagingbufferrange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	stagingbufferrange.baseMipLevel = 0;
	stagingbufferrange.levelCount = 1;
	stagingbufferrange.baseArrayLayer = 0;
	stagingbufferrange.layerCount = 1;

	VkImageMemoryBarrier stagingbuffertransferbarrier{};
	stagingbuffertransferbarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	stagingbuffertransferbarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	stagingbuffertransferbarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	stagingbuffertransferbarrier.image = rdata.rdtextureimage;
	stagingbuffertransferbarrier.subresourceRange = stagingbufferrange;
	stagingbuffertransferbarrier.srcAccessMask = 0;
	stagingbuffertransferbarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;



	VkExtent3D texextent{};
	texextent.width = static_cast<uint32_t>(w);
	texextent.height = static_cast<uint32_t>(h);
	texextent.depth = 1;

	VkBufferImageCopy stagingbuffercopy{};
	stagingbuffercopy.bufferOffset = 0;
	stagingbuffercopy.bufferRowLength = 0;
	stagingbuffercopy.bufferImageHeight = 0;
	stagingbuffercopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	stagingbuffercopy.imageSubresource.mipLevel = 0;
	stagingbuffercopy.imageSubresource.baseArrayLayer = 0;
	stagingbuffercopy.imageSubresource.layerCount = 1;
	stagingbuffercopy.imageExtent = texextent;





	VkImageMemoryBarrier stagingbuffershaderbarrier{};
	stagingbuffershaderbarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	stagingbuffershaderbarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	stagingbuffershaderbarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	stagingbuffershaderbarrier.image = rdata.rdtextureimage;
	stagingbuffershaderbarrier.subresourceRange = stagingbufferrange;
	stagingbuffershaderbarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	stagingbuffershaderbarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;



	if (vkResetCommandBuffer(stagingcommandbuffer, 0) != VK_SUCCESS) {
		logger::log(1, "%s error: failed to reset staging command buffer\n", __FUNCTION__);
		return false;
	}



	VkCommandBufferBeginInfo cmdbegininfo{};
	cmdbegininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdbegininfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(stagingcommandbuffer, &cmdbegininfo) != VK_SUCCESS) {
		logger::log(1, "%s error: failed to begin staging command buffer\n", __FUNCTION__);
		return false;
	}

	vkCmdPipelineBarrier(stagingcommandbuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &stagingbuffertransferbarrier);
	vkCmdCopyBufferToImage(stagingcommandbuffer, stagingbuffer, rdata.rdtextureimage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &stagingbuffercopy);
	vkCmdPipelineBarrier(stagingcommandbuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &stagingbuffershaderbarrier);


	if (vkEndCommandBuffer(stagingcommandbuffer) != VK_SUCCESS) {
		logger::log(1, "%s error: failed to end staging command buffer\n", __FUNCTION__);
		return false;
	}

	VkSubmitInfo submitinfo{};
	submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitinfo.pWaitDstStageMask = nullptr;
	submitinfo.waitSemaphoreCount = 0;
	submitinfo.pWaitSemaphores = nullptr;
	submitinfo.signalSemaphoreCount = 0;
	submitinfo.pSignalSemaphores = nullptr;
	submitinfo.commandBufferCount = 1;
	submitinfo.pCommandBuffers = &stagingcommandbuffer;

	VkFence stagingbufferfence;

	VkFenceCreateInfo fenceinfo{};
	fenceinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceinfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	if (vkCreateFence(rdata.rdvkbdevice.device, &fenceinfo, nullptr, &stagingbufferfence) != VK_SUCCESS){
		logger::log(1, "%s error: failed to create staging buffer fence\n", __FUNCTION__);
		return false;
	}
	if (vkResetFences(rdata.rdvkbdevice.device, 1, &stagingbufferfence) != VK_SUCCESS){
		logger::log(1, "%s error: staging buffer fence reset failed\n", __FUNCTION__);
		return false;
	}
	if (vkQueueSubmit(rdata.rdgraphicsqueue, 1, &submitinfo, stagingbufferfence) != VK_SUCCESS){
		logger::log(1, "%s error: failed to submit staging buffer copy command buffer\n", __FUNCTION__);
		return false;
	}
	if (vkWaitForFences(rdata.rdvkbdevice.device,1, &stagingbufferfence,VK_TRUE,INT64_MAX) != VK_SUCCESS){
		logger::log(1, "%s error: waiting for staging buffer copy fence failed\n", __FUNCTION__);
		return false;
	}




	vkDestroyFence(rdata.rdvkbdevice.device, stagingbufferfence, nullptr);
	commandbuffer::cleanup(rdata, stagingcommandbuffer);
	vmaDestroyBuffer(rdata.rdallocator, stagingbuffer, stagingbufferalloc);

	VkImageViewCreateInfo texviewinfo{};
	texviewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	texviewinfo.image = rdata.rdtextureimage;
	texviewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	texviewinfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	texviewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	texviewinfo.subresourceRange.baseMipLevel = 0;
	texviewinfo.subresourceRange.levelCount = 1;
	texviewinfo.subresourceRange.baseArrayLayer = 0;
	texviewinfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(rdata.rdvkbdevice.device, &texviewinfo, nullptr, &rdata.rdtextureimageview) != VK_SUCCESS)
	{
		logger::log(1, "%s error: could not create image view for texture\n", __FUNCTION__);
		return false;
	}

	VkSamplerCreateInfo texsamplerinfo{};
	texsamplerinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	texsamplerinfo.magFilter = VK_FILTER_LINEAR;
	texsamplerinfo.minFilter = VK_FILTER_LINEAR;
	texsamplerinfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	texsamplerinfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	texsamplerinfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	texsamplerinfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	texsamplerinfo.unnormalizedCoordinates = VK_FALSE;
	texsamplerinfo.compareEnable = VK_FALSE;
	texsamplerinfo.compareOp = VK_COMPARE_OP_ALWAYS;
	texsamplerinfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	texsamplerinfo.mipLodBias = 0.0f;
	texsamplerinfo.minLod = 0.0f;
	texsamplerinfo.maxLod = 0.0f;
	texsamplerinfo.anisotropyEnable = VK_FALSE;
	texsamplerinfo.maxAnisotropy = 1.0f;


	if (vkCreateSampler(rdata.rdvkbdevice.device, &texsamplerinfo, nullptr, &rdata.rdtexturesampler) != VK_SUCCESS) {
		logger::log(1, "%s error: could not create sampler for texture\n", __FUNCTION__);
		return false;
	}

	VkDescriptorSetLayoutBinding texturebind{};
	texturebind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	texturebind.binding = 0;
	texturebind.descriptorCount = 1;
	texturebind.pImmutableSamplers = nullptr;
	texturebind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo texcreateinfo{};
	texcreateinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	texcreateinfo.bindingCount = 1;
	texcreateinfo.pBindings = &texturebind;

	if (vkCreateDescriptorSetLayout(rdata.rdvkbdevice.device, &texcreateinfo, nullptr, &rdata.rdtexturelayout) != VK_SUCCESS) {
		logger::log(1, "%s error: could not create descriptor set layout\n", __FUNCTION__);
		return false;
	}


	VkDescriptorPoolSize poolsize{};
	poolsize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolsize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorpool{};
	descriptorpool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorpool.poolSizeCount = 1;
	descriptorpool.pPoolSizes = &poolsize;
	descriptorpool.maxSets = 16;

	if (vkCreateDescriptorPool(rdata.rdvkbdevice.device, &descriptorpool, nullptr, &rdata.rddescriptorpool) != VK_SUCCESS) {
		logger::log(1, "%s error: could not create descriptor pool\n", __FUNCTION__);
		return false;
	}


	VkDescriptorSetAllocateInfo descallocinfo{};
	descallocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descallocinfo.descriptorPool = rdata.rddescriptorpool;
	descallocinfo.descriptorSetCount = 1;
	descallocinfo.pSetLayouts = &rdata.rdtexturelayout;


	if (vkAllocateDescriptorSets(rdata.rdvkbdevice.device, &descallocinfo, &rdata.rddescriptorset) != VK_SUCCESS) {
		logger::log(1, "%s error: could not allocate descriptor set\n", __FUNCTION__);
		return false;
	}


	VkDescriptorImageInfo descriptorimginfo{};
	descriptorimginfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	descriptorimginfo.imageView = rdata.rdtextureimageview;
	descriptorimginfo.sampler = rdata.rdtexturesampler;

	VkWriteDescriptorSet writedescset{};
	writedescset.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writedescset.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writedescset.dstSet = rdata.rddescriptorset;
	writedescset.dstBinding = 0;
	writedescset.descriptorCount = 1;
	writedescset.pImageInfo = &descriptorimginfo;

	vkUpdateDescriptorSets(rdata.rdvkbdevice.device, 1, &writedescset, 0, nullptr);

	return true;

}












void texture::cleanup(vkobjs& rdata) {
	vkDestroyDescriptorPool(rdata.rdvkbdevice.device, rdata.rddescriptorpool, nullptr);
	vkDestroyDescriptorSetLayout(rdata.rdvkbdevice.device, rdata.rdtexturelayout, nullptr);
	vkDestroySampler(rdata.rdvkbdevice.device, rdata.rdtexturesampler, nullptr);
	vkDestroyImageView(rdata.rdvkbdevice.device, rdata.rdtextureimageview, nullptr);
	vmaDestroyImage(rdata.rdallocator, rdata.rdtextureimage, rdata.rdtextureimagealloc);
}