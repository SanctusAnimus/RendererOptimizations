#pragma once
#ifndef LIGHTSOURCE_CLASS_DECL
#define LIGHTSOURCE_CLASS_DECL

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "LightData.h"


class LightSource
{
public:
	LightSource(Rendering::LightData preset);
	LightSource(glm::vec3 position, glm::vec3 color);

	void Render();

	bool IsVisibleFrom(glm::mat4& mvp);
	Rendering::LightData GetData() { return data; };
	Rendering::LightData data;
private:
};

#endif // !LIGHTSOURCE_CLASS_DECL


