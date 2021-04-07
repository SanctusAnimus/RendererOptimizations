#pragma once
#ifndef LAB2SCENE_DECL
#define LAB2SCENE_DECL

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "BaseScene.h"
#include "../Renderer/Shader.h"
#include "../Renderer/Texture.h"
#include "../Renderer/Model/Model.h"
#include "../Renderer/Vertex.h"

class Lab2Scene : public BaseScene
{
public:
	Lab2Scene() : cube("Models/Crate/Crate1.obj") {
	}

	~Lab2Scene();

	void Setup() override;
	void Render() override;

	void BuildMesh();
	void BuildGrid();
	void BuildAxes();
	void BuildPlot();
private:
	unsigned int m_VAO = 0;
	unsigned int m_VBO;

	unsigned int m_GridVAO = 0;
	unsigned int m_GridVBO;

	unsigned int m_AxesVAO = 0;
	unsigned int m_AxesVBO;

	unsigned int m_PlotVAO = 0;
	unsigned int m_PlotVBO;

	unsigned int plot_fbo = 0;
	unsigned int plot_texture;

	int tess_outer = 10;
	int tess_inner = 10;

	std::shared_ptr<Shader> tess_shader;
	std::shared_ptr<Shader> points_shader;
	std::shared_ptr<Shader> axes_shader;
	std::shared_ptr<Shader> plot_shader;

	glm::vec3 translation;

	std::vector<BasicVertex> mesh;
	std::vector<BasicVertex> grid;
	std::vector<ColoredVertex> axes;
	std::vector<ColoredVertex> plot;

	bool wireframe = false;

	// plot params
	int m_PlotR = 150;
	int m_Plot_r = 30;
	float m_Tstep = 1.0;
	float m_Tmin = 0.0;
	float m_Tmax = 360.0;

	Model cube;
};

#endif // !LAB2SCENE_DECL


