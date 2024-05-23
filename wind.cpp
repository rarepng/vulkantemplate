#include "wind.h"
#include <iostream>
#include "logger.h"

bool wind::init(unsigned int w, unsigned int h, std::string title) {
	if (!glfwInit()) {
		logger::log(1, "%s error: glfwInit() failed\n", __FUNCTION__);
		return false;
	}
	if (!glfwVulkanSupported()) {
		glfwTerminate();
		logger::log(1, "%s error: Vulkan is not supported\n", __FUNCTION__);
		std::cout << "vulkan not supported";
		return false;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	mwind = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);

	if (!mwind) {
		logger::log(1, "%s error: Could not create window\n", __FUNCTION__);
		glfwTerminate();
		return false;
	}

	mvkrenderer = std::make_unique<vkrenderer>(mwind);
	if (!mvkrenderer->init(w, h)) {
		glfwTerminate();
		logger::log(1, "%s error: Could not init Vulkan\n", __FUNCTION__);
		return false;
	}
	glfwSetWindowUserPointer(mwind, mvkrenderer.get());
	glfwSetWindowSizeCallback(mwind, [](GLFWwindow* win, int width, int height) {
		auto renderer = static_cast<vkrenderer*>(glfwGetWindowUserPointer(win));
		renderer->setsize(width, height);
		});

	mmodel = std::make_unique<model>();
	mmodel->init();

	return true;

}

void wind::frameupdate() {
	mvkrenderer->uploaddata(mmodel->getmesh());
	while (!glfwWindowShouldClose(mwind)) {
		if (!mvkrenderer->draw()) {
			break;
		}
		glfwPollEvents();
	}
}

void wind::cleanup() {
	mvkrenderer->cleanup();
	glfwDestroyWindow(mwind);
	glfwTerminate();
}