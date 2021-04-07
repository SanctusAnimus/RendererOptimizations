#pragma once
#ifndef CUBE_CLASS_DECL
#define CUBE_CLASS_DECL

#include "../Renderable.h"


class Cube :
    public Renderable
{
public:
	Cube(std::shared_ptr<Texture> texture, std::shared_ptr<Shader> shader);

	void Render(std::shared_ptr<Camera::BaseCamera> camera, glm::mat4 projection) override;

	void SetNormalMap(std::string tex_name);
	void SetSpecularMap(std::string tex_name);
	void SetHeightMap(std::string tex_name);
private:
	unsigned int m_VAO = 0;
	unsigned int m_VBO;
	unsigned int m_EBO;

	std::shared_ptr<Texture> m_Texture;
	std::shared_ptr<Texture> m_NormalMap = nullptr;
	std::shared_ptr<Texture> m_SpecularMap = nullptr;
	std::shared_ptr<Texture> m_HeightMap = nullptr;
};

#endif // !CUBE_CLASS_DECL



