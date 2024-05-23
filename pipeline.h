#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include "vkobjs.h"
class pipeline {
public:
	static bool init(vkobjs& rdata,std::string vshade,std::string fshade);
	static void cleanup(vkobjs& rdata);
};