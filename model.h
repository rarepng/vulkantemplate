#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "vkobjs.h"
#include "vkrenderer.h"

class model {
public:
	void init();
	vkmesh getmesh();
private:
	vkmesh mmesh{ };
};