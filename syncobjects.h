#pragma once
#include <vulkan/vulkan.h>
#include "vkobjs.h"
class syncobjects {
public:
	static bool init(vkobjs& rdata);
	static void cleanup(vkobjs& rdata);
};