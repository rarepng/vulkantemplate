#include "model.h"

void model::init() {
	mmesh.verts.resize(6);

	mmesh.verts[0].pos = glm::vec3{-0.5f, -0.5f, 0.5f};
	mmesh.verts[1].pos = glm::vec3{0.5f, 0.5f, 0.5f};
	mmesh.verts[2].pos = glm::vec3{-0.5f, 0.5f, 0.5f};
	mmesh.verts[3].pos = glm::vec3{-0.5f, -0.5f, 0.5f};
	mmesh.verts[4].pos = glm::vec3{0.5f, -0.5f, 0.5f};
	mmesh.verts[5].pos = glm::vec3{0.5f, 0.5f, 0.5f};


	mmesh.verts[0].uv = glm::vec2{ 0.0, 0.0 };
	mmesh.verts[1].uv = glm::vec2{1.0, 1.0};
	mmesh.verts[2].uv = glm::vec2{0.0, 1.0};
	mmesh.verts[3].uv = glm::vec2{0.0, 0.0};
	mmesh.verts[4].uv = glm::vec2{1.0, 0.0};
	mmesh.verts[5].uv = glm::vec2{1.0, 1.0};

}

vkmesh model::getmesh() {
	return mmesh;
}