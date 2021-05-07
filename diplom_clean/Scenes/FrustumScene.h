#pragma once
#ifndef FRUSTUMSCENE_CLASS_DECL
#define FRUSTUMSCENE_CLASS_DECL

#include "BaseScene.h"
#include "../Renderer/Model/Model.h"
#include "../Renderer/Model/InstancedModel.h"
#include "../Renderer/LightData.h"
#include "../Renderer/LightSource.h"
#include "../Renderer/Buffers/GLTexture.h"
#include "../Renderer/Quad/Quad.h"

// #include <entt/entt.hpp>

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

class FrustumScene : public BaseScene
{
public:
	FrustumScene()
	{
		std::cout << "Scene constructor called!" << std::endl;
	};
	~FrustumScene();

	void Setup() override;
	void Render() override;
private:
	unsigned int m_GeometryFBO;
	GLTexture m_GeometryPosition, m_GeometryNormals, m_GeometryAlbedoSpec;
	unsigned int m_SSBO;
	unsigned int m_DepthRBO;

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
	std::shared_ptr<Shader> lighting_pass_shader;
	std::shared_ptr<Shader> light_box_shader;
	std::shared_ptr<Shader> blur_shader;
	std::shared_ptr<Shader> final_shader;
	std::shared_ptr<Shader> skybox_shader;

	std::shared_ptr<Shader> simple_tex_shader;
	std::shared_ptr<Shader> simple_g_shader;

	std::shared_ptr<Texture> skybox;
};

#endif // !FRUSTUMSCENE_CLASS_DECL