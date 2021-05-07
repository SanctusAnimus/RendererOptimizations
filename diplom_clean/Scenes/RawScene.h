#pragma once
#ifndef RAWSCENE_CLASS_DECL
#define RAWSCENE_CLASS_DECL

#include "BaseScene.h"

#include <vector>
#include <string>


class RawScene : public BaseScene
{
public:
	RawScene()
	{
		std::cout << "Scene constructor called!" << std::endl;
	};
	~RawScene();

	void Setup() override;
	void Render() override;
private:
	unsigned int m_SSBO;

	unsigned int m_hdrFBO;
	unsigned int m_hdrColorBuffers[2];
	unsigned int m_PingPongFBO[2];
	unsigned int m_PingPongColorBuffers[2];

	unsigned int m_SkyboxVAO;
	unsigned int m_SkyboxVBO;

	unsigned int m_FinalFBO;
	GLTexture m_FinalTexture;

	std::shared_ptr<Shader> geometry_pass_shader;
	std::shared_ptr<Shader> instanced_gp_shader;
	std::shared_ptr<Shader> blur_shader;
	std::shared_ptr<Shader> final_shader;

	std::vector<glm::mat4> positions;

	Model m_CubeModel;
	Model m_Backpack;
};

#endif // !INSTANCINGSCENE_CLASS_DECL



