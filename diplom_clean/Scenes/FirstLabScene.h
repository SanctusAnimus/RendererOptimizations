#pragma once
#ifndef LAB1SCENE_DECL
#define LAB1SCENE_DECL

#include <vector>

#include "BaseScene.h"
#include "../Renderer/Shader.h"
#include "../Renderer/Vertex.h"
#include "../Renderer/Model/Model.h"

class Lab1Scene : public BaseScene
{
public:
	Lab1Scene() : cube("Models/Crate/Crate1.obj") {};
	~Lab1Scene();

	void Setup() override;
	void Render() override;

	void BuildMesh();
	void BuildGrid();
	void BuildAxes();
	void BuildCurve();
private:
	unsigned int m_VAO = 0;
	unsigned int m_VBO;

	unsigned int m_GridVAO = 0;
	unsigned int m_GridVBO;

	unsigned int m_AxesVAO = 0;
	unsigned int m_AxesVBO;

	Shader* light_box_shader;
	Shader* points_shader;
	Shader* axes_shader;
	Shader* normals_shader;

	std::vector<BasicVertex> mesh;
	std::vector<BasicVertex> grid;
	std::vector<ColoredVertex> axes;

	std::vector<glm::vec3> source_verts;
	std::vector<glm::vec3> curve_cp;
	float source_step = 5;

	float curve_step = 0.1;

	Model cube;

	bool wireframe = false;
	bool normals = false;
};

#endif // !LAB1SCENE_DECL
