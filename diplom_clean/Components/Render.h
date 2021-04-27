#pragma once

#ifndef RENDER_COMPONENT_DECL
#define RENDER_COMPONENT_DECL

#include <glm/glm.hpp>
#include <string>


struct QuadComponent {

};

struct InstancedQuadComponent {

};

struct ModelComponent {

};

struct InstancedModelComponent {
	glm::vec4 m_Color;
	std::string m_ModelName;
	std::string m_ShaderName;
	int m_InstanceIdx = -1;

	InstancedModelComponent(const std::string& model_name, const std::string& shader_name) 
		: m_ModelName(model_name), m_ShaderName(shader_name)
	{

	}

	void SetRenderColor(const glm::vec4& color) {
		m_Color = color;
	}

	void SetModel(const std::string& name) {
		m_ModelName = name;
	}
};

#endif // !RENDER_COMPONENT_DECL


