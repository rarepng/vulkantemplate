#include <cstring>
#define VMA_IMPLEMENTATION
#include "vk/vk_mem_alloc.h"
#include "vkrenderer.h"
#include <iostream>
#include "logger.h"

vkrenderer::vkrenderer(GLFWwindow* wind) {
	mwind = wind;
}

bool vkrenderer::init(unsigned int w, unsigned int h) {
	if (!mwind) {
		logger::log(1, "%s error: invalid GLFWwindow handle\n", __FUNCTION__);
		return false;
	}
	if (!deviceinit())return false;
	if (!initvma())return false;
	if (!getqueue())return false;
	if (!createswapchain())return false;
	if (!createdepthbuffer())return false;
	if (!createcommandpool())return false;
	if (!createcommandbuffer())return false;
	if (!loadtexs())return false;
	if (!createrenderpass())return false;
	if (!createpipeline())return false;
	if (!createframebuffer())return false;
	if (!createsyncobjects())return false;

	return true;
}

bool vkrenderer::deviceinit() {
	vkb::InstanceBuilder instbuild;
	auto instret = instbuild.use_default_debug_messenger().request_validation_layers().build();
	mvkobjs.rdvkbinstance = instret.value();

	VkResult res = VK_ERROR_UNKNOWN;
	res = glfwCreateWindowSurface(mvkobjs.rdvkbinstance, mwind, nullptr, &msurface);

	vkb::PhysicalDeviceSelector physicaldevsel{ mvkobjs.rdvkbinstance };
	auto physicaldevselret = physicaldevsel.set_surface(msurface).select();
	mphysdev = physicaldevselret.value();

	vkb::DeviceBuilder devbuilder{ mphysdev };
	auto devbuilderret = devbuilder.build();
	mvkobjs.rdvkbdevice = devbuilderret.value();
	
	return true;
}

bool vkrenderer::getqueue()
{
	auto graphqueueret = mvkobjs.rdvkbdevice.get_queue(vkb::QueueType::graphics);
	mvkobjs.rdgraphicsqueue = graphqueueret.value();

	auto presentqueueret = mvkobjs.rdvkbdevice.get_queue(vkb::QueueType::present);
	mvkobjs.rdpresentqueue = presentqueueret.value();

	return true;
}


bool vkrenderer::createdepthbuffer() {
	VkExtent3D depthimageextent = {
		mvkobjs.rdvkbswapchain.extent.width,
		mvkobjs.rdvkbswapchain.extent.height,
		1
	};

	mvkobjs.rddepthformat = VK_FORMAT_D32_SFLOAT;

	VkImageCreateInfo depthimginfo{};
	depthimginfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthimginfo.imageType = VK_IMAGE_TYPE_2D;
	depthimginfo.format = mvkobjs.rddepthformat;
	depthimginfo.extent = depthimageextent;
	depthimginfo.mipLevels = 1;
	depthimginfo.arrayLayers = 1;
	depthimginfo.samples = VK_SAMPLE_COUNT_1_BIT;
	depthimginfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthimginfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VmaAllocationCreateInfo depthallocinfo{};
	depthallocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	depthallocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vmaCreateImage(mvkobjs.rdallocator, &depthimginfo, &depthallocinfo, &mvkobjs.rddepthimage, &mvkobjs.rddepthimagealloc, nullptr) != VK_SUCCESS) {
		return false;
	}

	VkImageViewCreateInfo depthimgviewinfo{};
	depthimgviewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthimgviewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthimgviewinfo.image = mvkobjs.rddepthimage;
	depthimgviewinfo.format = mvkobjs.rddepthformat;
	depthimgviewinfo.subresourceRange.baseMipLevel = 0;
	depthimgviewinfo.subresourceRange.levelCount = 1;
	depthimgviewinfo.subresourceRange.baseArrayLayer = 0;
	depthimgviewinfo.subresourceRange.layerCount = 1;
	depthimgviewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	if (vkCreateImageView(mvkobjs.rdvkbdevice, &depthimgviewinfo, nullptr, &mvkobjs.rddepthimageview) != VK_SUCCESS) {
		return false;
	}


	return true;
}






bool vkrenderer::createswapchain() {
	vkb::SwapchainBuilder swapchainbuild{ mvkobjs.rdvkbdevice };
	auto swapchainbuilret = swapchainbuild.set_old_swapchain(mvkobjs.rdvkbswapchain).set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR).build();
	if (!swapchainbuilret) {
		return false;
	}
	vkb::destroy_swapchain(mvkobjs.rdvkbswapchain);
	mvkobjs.rdvkbswapchain = swapchainbuilret.value();
	return true;
}

bool vkrenderer::recreateswapchain() {
	int w = 0, h = 0;
	glfwGetFramebufferSize(mwind, &w, &h);
	while (w == 0 || h == 0) {
		glfwGetFramebufferSize(mwind, &w, &h);
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(mvkobjs.rdvkbdevice.device);





	framebuffer::cleanup(mvkobjs);
	vkDestroyImageView(mvkobjs.rdvkbdevice.device, mvkobjs.rddepthimageview, nullptr);
	vmaDestroyImage(mvkobjs.rdallocator, mvkobjs.rddepthimage, mvkobjs.rddepthimagealloc);

	mvkobjs.rdvkbswapchain.destroy_image_views(mvkobjs.rdswapchainimageviews);




	if (!createswapchain()) {
		return false;
	}if (!createdepthbuffer()) {
		return false;
	}if (!createframebuffer()) {
		return false;
	}




	return true;
}

bool vkrenderer::createrenderpass() {
	if (!renderpass::init(mvkobjs))return false;
	return true;
}


bool vkrenderer::createpipeline() {
	std::string vfile = "shader/basic.vert.spv";
	std::string ffile = "shader/basic.frag.spv";
	if (!pipeline::init(mvkobjs, vfile, ffile)) { return false; std::cout << std::endl << " failed to create shaders" << std::endl; }
	return true;
}


bool vkrenderer::createframebuffer() {
	if (!framebuffer::init(mvkobjs))return false;
	return true;
}
bool vkrenderer::createcommandpool() {
	if (!commandpool::init(mvkobjs))return false;
	return true;
}
bool vkrenderer::createcommandbuffer() {
	if (!commandbuffer::init(mvkobjs,mvkobjs.rdcommandbuffer))return false;
	return true;
}
bool vkrenderer::createsyncobjects() {
	if (!syncobjects::init(mvkobjs))return false;
	return true;
}
bool vkrenderer::loadtexs() {
	std::string tfile = "textures/crate.png";
	if (!texture::loadtexture(mvkobjs,tfile))return false;
	return true;
}



bool vkrenderer::initvma() {
	VmaAllocatorCreateInfo allocinfo{};
	allocinfo.physicalDevice = mphysdev.physical_device;
	allocinfo.device = mvkobjs.rdvkbdevice.device;
	allocinfo.instance = mvkobjs.rdvkbinstance.instance;
	if (vmaCreateAllocator(&allocinfo, &mvkobjs.rdallocator) != VK_SUCCESS) {
		return false;
	}
	return true;
}

void vkrenderer::cleanup() {
	vkDeviceWaitIdle(mvkobjs.rdvkbdevice.device);
	texture::cleanup(mvkobjs);
	vmaDestroyBuffer(mvkobjs.rdallocator, mvertexbuffer, mvertexbufferalloc);

	texture::cleanup(mvkobjs);
	syncobjects::cleanup(mvkobjs);
	commandbuffer::cleanup(mvkobjs,mvkobjs.rdcommandbuffer);
	commandpool::cleanup(mvkobjs);
	framebuffer::cleanup(mvkobjs);
	pipeline::cleanup(mvkobjs);
	renderpass::cleanup(mvkobjs);

	vkDestroyImageView(mvkobjs.rdvkbdevice.device, mvkobjs.rddepthimageview, nullptr);
	vmaDestroyImage(mvkobjs.rdallocator, mvkobjs.rddepthimage, mvkobjs.rddepthimagealloc);
	vmaDestroyAllocator(mvkobjs.rdallocator);

	mvkobjs.rdvkbswapchain.destroy_image_views(mvkobjs.rdswapchainimageviews);
	vkb::destroy_swapchain(mvkobjs.rdvkbswapchain);

	vkb::destroy_device(mvkobjs.rdvkbdevice);
	vkb::destroy_surface(mvkobjs.rdvkbinstance.instance, msurface);
	vkb::destroy_instance(mvkobjs.rdvkbinstance);


}

void vkrenderer::setsize(unsigned int w, unsigned int h) {
	std::cout << "size changed";
}



bool vkrenderer::uploaddata(vkmesh mesh) {
	VkBufferCreateInfo bufferinfo{};
	bufferinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferinfo.size = mesh.verts.size()*sizeof(vkvert);
	bufferinfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	VmaAllocationCreateInfo vmallocinfo{};
	vmallocinfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	std::cout << std::endl << mesh.verts.size() << std::endl << sizeof(vkvert) << std::endl << std::endl;
	if (vmaCreateBuffer(mvkobjs.rdallocator, &bufferinfo, &vmallocinfo, &mvertexbuffer, &mvertexbufferalloc, nullptr) != VK_SUCCESS) {
		return false;
	}

	void* data;

	vmaMapMemory(mvkobjs.rdallocator, mvertexbufferalloc, &data);
	std::memcpy(data, mesh.verts.data(), mesh.verts.size() * sizeof(vkvert));
	vmaUnmapMemory(mvkobjs.rdallocator, mvertexbufferalloc);

	
	trianglecount = mesh.verts.size() / 3;

	return true;
}

bool vkrenderer::draw() {
	if (vkWaitForFences(mvkobjs.rdvkbdevice.device, 1, &mvkobjs.rdrenderfence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
		return false;
	}
	if (vkResetFences(mvkobjs.rdvkbdevice.device, 1, &mvkobjs.rdrenderfence) != VK_SUCCESS)return false;

	uint32_t imgidx = 0;
	VkResult res = vkAcquireNextImageKHR(mvkobjs.rdvkbdevice.device, mvkobjs.rdvkbswapchain.swapchain, UINT64_MAX, mvkobjs.rdpresentsemaphore, VK_NULL_HANDLE, &imgidx);
	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		return recreateswapchain();
	}
	else {
		if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
			return false;
		}
	}

	if (vkResetCommandBuffer(mvkobjs.rdcommandbuffer, 0) != VK_SUCCESS) {
		return false;
	}

	VkCommandBufferBeginInfo cmdbegininfo{};
	cmdbegininfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdbegininfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(mvkobjs.rdcommandbuffer, &cmdbegininfo) != VK_SUCCESS)return false;

	VkClearValue colorclearvalue;
	colorclearvalue.color = { {0.1f,0.1f,0.1f,1.0f } };

	VkClearValue depthvalue;
	depthvalue.depthStencil.depth = 1.0f;

	VkClearValue clearvals[] = { colorclearvalue,depthvalue };

	
	VkRenderPassBeginInfo rpinfo{};
	rpinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpinfo.renderPass = mvkobjs.rdrenderpass;

	rpinfo.renderArea.offset.x = 0;
	rpinfo.renderArea.offset.y = 0;
	rpinfo.renderArea.extent = mvkobjs.rdvkbswapchain.extent;
	rpinfo.framebuffer = mvkobjs.rdframebuffers[imgidx];

	rpinfo.clearValueCount = 2;
	rpinfo.pClearValues = clearvals;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(mvkobjs.rdvkbswapchain.extent.width);
	viewport.height = static_cast<float>(mvkobjs.rdvkbswapchain.extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0,0 };
	scissor.extent = mvkobjs.rdvkbswapchain.extent;

	vkCmdBeginRenderPass(mvkobjs.rdcommandbuffer, &rpinfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(mvkobjs.rdcommandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mvkobjs.rdpipeline);

	vkCmdSetViewport(mvkobjs.rdcommandbuffer, 0, 1, &viewport);
	vkCmdSetScissor(mvkobjs.rdcommandbuffer, 0, 1, &scissor);

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(mvkobjs.rdcommandbuffer, 0, 1, &mvertexbuffer, &offset);
	vkCmdBindDescriptorSets(mvkobjs.rdcommandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mvkobjs.rdpipelinelayout, 0, 1, &mvkobjs.rddescriptorset, 0, nullptr);

	vkCmdDraw(mvkobjs.rdcommandbuffer, trianglecount * 3, 1, 0, 0);

	vkCmdEndRenderPass(mvkobjs.rdcommandbuffer);

	if (vkEndCommandBuffer(mvkobjs.rdcommandbuffer) != VK_SUCCESS)return false;

	VkSubmitInfo submitinfo{};
	submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags waitstage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitinfo.pWaitDstStageMask = &waitstage;

	submitinfo.waitSemaphoreCount = 1;
	submitinfo.pWaitSemaphores = &mvkobjs.rdpresentsemaphore;

	submitinfo.signalSemaphoreCount = 1;
	submitinfo.pSignalSemaphores = &mvkobjs.rdrendersemaphore;

	submitinfo.commandBufferCount = 1;
	submitinfo.pCommandBuffers = &mvkobjs.rdcommandbuffer;


	if (vkQueueSubmit(mvkobjs.rdgraphicsqueue, 1, &submitinfo, mvkobjs.rdrenderfence) != VK_SUCCESS) {
		return false;
	}

	VkPresentInfoKHR presentinfo{};
	presentinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentinfo.waitSemaphoreCount = 1;
	presentinfo.pWaitSemaphores = &mvkobjs.rdrendersemaphore;

	presentinfo.swapchainCount = 1;
	presentinfo.pSwapchains = &mvkobjs.rdvkbswapchain.swapchain;

	presentinfo.pImageIndices = &imgidx;

	res = vkQueuePresentKHR(mvkobjs.rdpresentqueue, &presentinfo);
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
		return recreateswapchain();
	}
	else {
		if (res != VK_SUCCESS) {
			return false;
		}
	}




	return true;
}