#pragma once
#include <vulkan/vulkan.h>
#include "vkobjs.h"
class commandbuffer {
public:
	static bool init(vkobjs& rdata,VkCommandBuffer &incommandbuffer);
	static void cleanup(vkobjs& rdata, VkCommandBuffer& incommandbuffer);
};