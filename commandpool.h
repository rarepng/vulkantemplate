#pragma once
#include <vulkan/vulkan.h>
#include "vkobjs.h"
class commandpool {
public:
	static bool init(vkobjs& rdata);
	static void cleanup(vkobjs& rdata);
};