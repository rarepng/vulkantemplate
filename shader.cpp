#include <fstream>
#include <cerrno>
#include <cstring>

#include "shader.h"
#include "logger.h"

VkShaderModule shader::loadshader(VkDevice dev, std::string filename) {
	std::string shadertxt;
	shadertxt = loadfiletostr(filename);

	VkShaderModuleCreateInfo shadercreateinfo{};
	shadercreateinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shadercreateinfo.codeSize = shadertxt.size();
	shadercreateinfo.pCode = reinterpret_cast<const uint32_t*>(shadertxt.c_str());

	VkShaderModule shadermod;
	if (vkCreateShaderModule(dev, &shadercreateinfo, nullptr, &shadermod) != VK_SUCCESS) {
		logger::log(1, "%s: could not load shader '%s'\n", __FUNCTION__, filename.c_str());
		return VK_NULL_HANDLE;
	}
	return shadermod;
}
std::string shader::loadfiletostr(std::string filename) {
	std::ifstream infile(filename, std::ios::binary);
	std::string str;

	if (infile.is_open()) {
		str.clear();
		infile.seekg(0, std::ios::end);
		str.reserve(infile.tellg());
		infile.seekg(0, std::ios::beg);

		str.assign((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
		infile.close();
	}
	else {
		logger::log(1, "%s error: could not open file %s\n", __FUNCTION__, filename.c_str());
		return std::string();
	}



	if (infile.bad() || infile.fail()) {
		logger::log(1, "%s error: error while reading file %s\n", __FUNCTION__, filename.c_str());
		return std::string();
	}






	infile.close();
	return str;
}