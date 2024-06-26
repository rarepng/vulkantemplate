#include "commandbuffer.h"
#include "logger.h"
#include <vk/VkBootstrap.h>

bool commandbuffer::init(vkobjs& rdata, VkCommandBuffer& incommandbuffer) {
	VkCommandBufferAllocateInfo bufferallocinfo{};
	bufferallocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	bufferallocinfo.commandPool = rdata.rdcommandpool;
	bufferallocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	bufferallocinfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(rdata.rdvkbdevice.device, &bufferallocinfo, &incommandbuffer) != VK_SUCCESS) {
		logger::log(1, "%s error: could not allocate command buffers\n", __FUNCTION__);
		return false;
	}

	return true;

}

void commandbuffer::cleanup(vkobjs& rdata, VkCommandBuffer& incommandbuffer) {
	vkFreeCommandBuffers(rdata.rdvkbdevice.device, rdata.rdcommandpool, 1, &incommandbuffer);
}