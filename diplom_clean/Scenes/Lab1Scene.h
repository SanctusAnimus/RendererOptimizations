#pragma once
#ifndef LAB1SCENE_DECL
#define LAB1SCENE_DECL

#include <vector>

#include "BaseScene.h"
#include "../Renderer/Shader.h"
#include "../Renderer/Vertex.h"

class Lab1Scene : public BaseScene
{
public:
	void Setup() override;
	void Render() override;

	void BuildMesh();
private:
	unsigned int m_VAO = 0;
	unsigned int m_VBO;

	Shader* light_box_shader;
	std::vector<BasicVertex> mesh;

	bool params_changed = true;
};

#endif // !LAB1SCENE_DECL
