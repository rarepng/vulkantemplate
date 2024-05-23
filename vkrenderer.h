#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vk/VkBootstrap.h>
#include <vk/vk_mem_alloc.h>

#include "vkobjs.h"
#include "renderpass.h"
#include "pipeline.h"
#include "framebuffer.h"
#include "commandpool.h"
#include "commandbuffer.h"
#include "syncobjects.h"
#include "texture.h"





class vkrenderer {
public:
	vkrenderer(GLFWwindow* wind);
	bool init(unsigned int w, unsigned int h);
	void setsize(unsigned int w, unsigned int h);
	bool draw();
	bool uploaddata(vkmesh mesh);
	void cleanup();

private:
	vkobjs mvkobjs{};
	int trianglecount = 0;
	GLFWwindow* mwind = nullptr;
	VkSurfaceKHR msurface = VK_NULL_HANDLE;
	vkb::PhysicalDevice mphysdev;
	VkBuffer mvertexbuffer;
	VmaAllocation mvertexbufferalloc;


	bool deviceinit();
	bool getqueue();
	bool createdepthbuffer();
	bool createswapchain();
	bool createrenderpass();
	bool createpipeline();
	bool createframebuffer();
	bool createcommandpool();
	bool createcommandbuffer();
	bool createsyncobjects();
	bool loadtexs();

	bool initvma();

	bool recreateswapchain();




};