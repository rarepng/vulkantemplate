#pragma once
#include <string>
#include <vulkan/vulkan.h>

class shader {
public:
	static VkShaderModule loadshader(VkDevice dev, std::string filename);
private:
	static std::string loadfiletostr(std::string filename);
};