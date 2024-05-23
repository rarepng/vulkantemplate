#pragma once
#include <vulkan/vulkan.h>
#include "vkobjs.h"
class texture {
public:
	static bool loadtexture(vkobjs& rdata,std::string texfile);
	static void cleanup(vkobjs& rdata);
};