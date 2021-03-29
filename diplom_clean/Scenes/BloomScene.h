#pragma once
#ifndef BLOOMSCENE_CLASS_DECL
#define BLOOMSCENE_CLASS_DECL

#include "../BaseScene.h"
#include "../Model.h"
#include "../LightData.h"
#include "../LightSource.h"

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

class BloomScene : public BaseScene
{
public:
	BloomScene()
		: backpack("Models/backpack/backpack.obj"), cube("Models/Crate/Crate1.obj") {
		std::cout << "Scene constructor called!" << std::endl;
	};
	void RegenerateLights();
	void Setup() override;
	void Render() override;
private:
	std::vector<glm::vec3> objectPositions;
	std::vector<LightSource> light_data;

	unsigned int gBuffer;
	unsigned int gPosition, gNormal, gAlbedoSpec;
	unsigned int ssbo;

	Shader* geometry_pass_shader;
	Shader* instanced_gp_shader;
	Shader* lighting_pass_shader;
	Shader* light_box_shader;

	Model backpack;
	Model cube;

	const int LIGHT_COUNT_MAX = 5;
};

#endif // !BLOOMSCENE_CLASS_DECL


