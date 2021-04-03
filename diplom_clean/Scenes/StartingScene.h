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

class StartingScene : public BaseScene
{
public:
	StartingScene()
		: backpack("Models/backpack/backpack.obj"), cube("Models/Crate/Crate1.obj") {
		std::cout << "Scene constructor called!" << std::endl;
	};
	~StartingScene();
	void RegenerateLights();
	void Setup() override;
	void Render() override;
private:
	std::vector<glm::vec3> objectPositions;
	std::vector<LightSource> light_data;

	unsigned int gBuffer;
	unsigned int gPosition, gNormal, gAlbedoSpec;
	unsigned int ssbo;

	unsigned int hdrFBO;
	unsigned int colorBuffers[2];
	unsigned int pingpongFBO[2];
	unsigned int pingpongColorbuffers[2];

	std::shared_ptr<Shader> geometry_pass_shader;
	std::shared_ptr<Shader> instanced_gp_shader;
	std::shared_ptr<Shader> lighting_pass_shader;
	std::shared_ptr<Shader> light_box_shader;
	std::shared_ptr<Shader> blur_shader;
	std::shared_ptr<Shader> final_shader;

	Model backpack;
	Model cube;

	float spread = 20.f;
	const int LIGHT_COUNT_MAX = 256;

	int count = 1;

	bool display_only_visible = true;
};

#endif // !STARTINGSCENE_CLASS_DECL



