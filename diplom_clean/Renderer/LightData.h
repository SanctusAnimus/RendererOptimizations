#pragma once
#ifndef LIGHTDATA_STRUCT_DECL
#define LIGHTDATA_STRUCT_DECL

#include <glm/glm.hpp>

namespace Rendering {
	struct alignas(16) LightData {
		glm::vec4 Position;
		glm::vec4 Color;

		float Linear;
		float Quadratic;
		float Radius;
	};
}

#endif // !LIGHTDATA_STRUCT_DECL
