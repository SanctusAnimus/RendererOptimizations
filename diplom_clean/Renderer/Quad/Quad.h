#pragma once
#ifndef QUAD_CLASS_DECL
#define QUAD_CLASS_DECL

#include "../Renderable.h"

class Quad : public Renderable
{
public:
	Quad(Texture* texture, std::shared_ptr<Shader> shader);

	void Render(std::shared_ptr<Camera::BaseCamera> camera, glm::mat4 projection) override;
	void UI_Description() override;
private:
	unsigned int m_VAO = 0;
	unsigned int m_VBO;
	unsigned int m_EBO;
};

#endif // !QUAD_CLASS_DECL