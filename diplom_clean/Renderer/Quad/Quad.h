#pragma once
#ifndef QUAD_CLASS_DECL
#define QUAD_CLASS_DECL

#include "../Renderable.h"
#include <glm/gtc/type_ptr.hpp>

class Quad : public Renderable
{
public:
	Quad(std::shared_ptr<Texture> texture, std::shared_ptr<Shader> shader);

	void Render(std::shared_ptr<Camera::BaseCamera> camera, glm::mat4 projection) override;
	void SetUV(glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3, glm::vec2 uv4);
	void UI_Description() override;

	void Rebuild();
private:

	unsigned int m_VAO = 0;
	unsigned int m_VBO;
	unsigned int m_EBO;

	glm::vec2 m_UV1 = glm::vec2(0.0f, 1.0f);
	glm::vec2 m_UV2 = glm::vec2(0.0f, 0.0f);
	glm::vec2 m_UV3 = glm::vec2(1.0f, 0.0f);
	glm::vec2 m_UV4 = glm::vec2(1.0f, 1.0f);
};

#endif // !QUAD_CLASS_DECL