#include <iostream>
#include <vk/Vkbootstrap.h>
#include <vk/vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "wind.h"
#include "logger.h"

int main() {

	std::unique_ptr<wind> w = std::make_unique<wind>();

	if (!w->init(640, 480, "vk!!!!!!!!!!")) {
		logger::log(1, "%s error: Window init error\n", __FUNCTION__);
		std::cout << "broken";
		return -1;
	}
	w->frameupdate();

	w->cleanup();
	std::cout << "HELL";
	return 0;
}