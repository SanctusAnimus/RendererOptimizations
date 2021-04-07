#pragma once
#ifndef INSTANCED_QUAD_CLASS_DECL
#define INSTANCED_QUAD_CLASS_DECL

#include "../Renderable.h"
#include <vector>

class InstancedQuad
{
public:
	InstancedQuad(std::shared_ptr<Texture> texture, std::shared_ptr<Shader> shader);
	void Add(glm::mat4 position);

	void Render(std::shared_ptr<Camera::BaseCamera> camera, glm::mat4 projection);
	void UI_Description();
	void SetNormalMap(std::string tex_name);
	void SetSpecularMap(std::string tex_name);
	void SetHeightMap(std::string tex_name);
private:
	bool m_BuffersInit = false;

	std::vector<glm::mat4> models; // !! this vector contains already transformed matrices !!

	unsigned int m_VAO = 0; // vertex array object
	unsigned int m_VBO;		// vertex buffer object
	unsigned int m_EBO;		// element buffer object
	unsigned int m_ABO;		// array buffer object

	unsigned int indices[6] = {
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	std::shared_ptr<Shader> m_Shader;
	std::shared_ptr<Texture> m_Texture;
	std::shared_ptr<Texture> m_NormalMap = nullptr;
	std::shared_ptr<Texture> m_SpecularMap = nullptr;
	std::shared_ptr<Texture> m_HeightMap = nullptr;
};

#endif // !INSTANCED_QUAD_CLASS_DECL