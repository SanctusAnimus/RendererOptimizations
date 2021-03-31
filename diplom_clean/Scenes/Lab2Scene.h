#pragma once
#ifndef LAB2SCENE_DECL
#define LAB2SCENE_DECL

#include <vector>

#include "BaseScene.h"
#include "../Renderer/Shader.h"
#include "../Renderer/Vertex.h"

class Lab2Scene : public BaseScene
{
public:
	void Setup() override;
	void Render() override;

	void RebuildMesh();
private:
	unsigned int m_VAO = 0;
	unsigned int m_VBO;

	int tess_outer = 5;
	int tess_inner = 5;

	Shader* tess_shader;

	std::vector<BasicVertex> mesh;

	bool params_changed = true;
};

#endif // !LAB2SCENE_DECL


