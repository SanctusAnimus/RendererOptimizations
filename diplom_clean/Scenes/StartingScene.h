#pragma once
#ifndef STARTINGSCENE_CLASS_DECL
#define STARTINGSCENE_CLASS_DECL

#include "BaseScene.h"
#include "../Renderer/Model/Model.h"
#include "../Renderer/LightData.h"
#include "../Renderer/LightSource.h"

// #include <entt/entt.hpp>

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <entt/entt.hpp>

class StartingScene : public BaseScene
{
public:
	StartingScene()
		: m_CubeModel("Models/Crate/Crate1.obj") {
		std::cout << "Scene constructor called!" << std::endl;
	};
	~StartingScene();
	void RegenerateLights();
	void Setup() override;
	void Render() override;

	entt::entity AddEntity() {
		return registry.create();
	}
private:
	std::vector<LightSource> m_LightData;

	unsigned int m_GeometryFBO;
	unsigned int m_GeometryPosition, m_GeometryNormals, m_GeometryAlbedoSpec;
	unsigned int m_SSBO;
	unsigned int m_DepthRBO;

	unsigned int m_hdrFBO;
	unsigned int m_hdrColorBuffers[2];
	unsigned int m_PingPongFBO[2];
	unsigned int m_PingPongColorBuffers[2];

	unsigned int m_SkyboxVAO;
	unsigned int m_SkyboxVBO;

	std::shared_ptr<Shader> geometry_pass_shader;
	std::shared_ptr<Shader> instanced_gp_shader;
	std::shared_ptr<Shader> lighting_pass_shader;
	std::shared_ptr<Shader> light_box_shader;
	std::shared_ptr<Shader> blur_shader;
	std::shared_ptr<Shader> final_shader;
	std::shared_ptr<Shader> skybox_shader;

	std::shared_ptr<Texture> skybox;

	Model m_CubeModel;

	float spread = 20.f;
	const int LIGHT_COUNT_MAX = 256;

	int m_ModelsStride = 3;

	entt::registry registry;
};

#endif // !STARTINGSCENE_CLASS_DECL



