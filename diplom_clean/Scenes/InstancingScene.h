#pragma once
#ifndef INSTANCINGSCENE_CLASS_DECL
#define INSTANCINGSCENE_CLASS_DECL

#include "BaseScene.h"

#include <vector>
#include <string>


class InstancingScene : public BaseScene
{
public:
	InstancingScene()
	{
		std::cout << "Scene constructor called!" << std::endl;
	};
	~InstancingScene();

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
};

#endif // !INSTANCINGSCENE_CLASS_DECL



