#pragma once
#include <string>
#include <memory>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "vkrenderer.h"
#include "model.h"

class wind {
public:
	bool init(unsigned int width, unsigned int height, std::string title);
	void frameupdate();
	void cleanup();
private:
	GLFWwindow* mwind = nullptr;
	std::unique_ptr<vkrenderer> mvkrenderer;
	std::unique_ptr<model> mmodel;
};