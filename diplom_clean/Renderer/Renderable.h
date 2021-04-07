#pragma once
#ifndef RENDERABLE_CLASS_DECL
#define RENDERABLE_CLASS_DECL

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>

#include "Camera/BaseCamera.h"
#include "Shader.h"
#include "Texture.h"

class Renderable {
public:
	Renderable(std::shared_ptr<Texture> texture, std::shared_ptr<Shader> shader)
		: m_Transform({ 0.f }), m_Scale({ 1.f }), m_Rotation({ 0.f }), m_RotationRadians(0), m_Texture(texture), m_Shader(shader)
	{};

	virtual void Render(std::shared_ptr<Camera::BaseCamera> camera, glm::mat4 projection) {};

	virtual void SetTransform(glm::vec3 transform) { m_Transform = transform; };
	virtual void SetScale(glm::vec3 scale) { m_Scale = scale; };
	virtual void SetRotation(float rotation_degree, glm::vec3 rotation) {
		m_Rotation = rotation; 
		m_RotationRadians = glm::radians(rotation_degree);
	};
	virtual void UI_Description() {};

	void SetNormalMap(std::string tex_name);
	void SetSpecularMap(std::string tex_name);
	void SetHeightMap(std::string tex_name);
protected:
	glm::vec3 m_Transform;
	glm::vec3 m_Scale;
	float m_RotationRadians;
	glm::vec3 m_Rotation;

	std::shared_ptr<Shader> m_Shader;
	std::shared_ptr<Texture> m_Texture;
	std::shared_ptr<Texture> m_NormalMap = nullptr;
	std::shared_ptr<Texture> m_SpecularMap = nullptr;
	std::shared_ptr<Texture> m_HeightMap = nullptr;
};


#endif // !RENDERABLE_CLASS_DECL