#pragma once
#ifndef RENDERER_CLASS_DECL
#define RENDERER_CLASS_DECL

#include <vector>
#include <map>

#include "LightData.h"
#include "Texture.h"
#include "Model/Model.h"
#include "Model/InstancedModel.h"
#include "Quad/Quad.h"
#include "Quad/InstancedQuad.h"
#include "Camera/BaseCamera.h"
#include "Camera/FlyCamera.h"
#include "Camera/ArcballCamera.h"

namespace Rendering {
	extern unsigned int SCREEN_WIDTH;
	extern unsigned int SCREEN_HEIGHT;


	struct RenderSettings {
		int light_count = 32;
		float light_spread = 6;

		float light_constant = 1.0;
		float light_linear = 0.7;
		float light_quadratic = 1.4;

		bool wireframe = false;
		bool gamma_correction = false;

		float models_sphere_radius = 1.0;
	};
};

class Renderer
{
public:
	Renderer();

	Model& NewModel();
	std::shared_ptr<Quad> NewQuad(std::string texture_name, std::string shader_name);
	std::shared_ptr<InstancedQuad> NewInstancedQuad(std::string texture_name, std::string shader_name, std::string instance_name);

	std::shared_ptr<InstancedModel> NewInstancedModel(std::string model_path, std::string shader_name, std::string instance_name);

	// basic VS + FS shader
	Shader* NewShader(const char* vertex_path, const char* fragment_path, std::string name);
	// VS + FS + GS
	Shader* NewShader(const char* vertex_path, const char* fragment_path, const char* geometry_pass, std::string name);
	// VS + FS + TCS + TES + GS
	Shader* NewShader(const char* vertex_path, const char* fragment_path, const char* tess_control_path, const char* tess_eval_path, std::string name);


	Texture* NewTexture(const char* file_path, std::string name, std::string type);

	std::shared_ptr<Camera::BaseCamera> NewCamera(glm::vec3 coords, std::string camera_name, Camera::Camera_Type cam_type);

	Shader* GetShader(std::string shader_name);
	Texture* GetTexture(std::string texture_name);
	std::shared_ptr<InstancedQuad> GetInstancedQuad(std::string instance_name);

	static Renderer& instance()
	{
		static Renderer s_instance;
		return s_instance;
	};

	void Render();
	void GatherImGui();

	void SimpleQuad();

	void SetActiveCamera(std::string camera_name);

	std::shared_ptr<Camera::BaseCamera> current_camera;
	Camera::Camera_Type current_camera_type;
	float frame_time = 0.1f;
	Rendering::RenderSettings settings;
	unsigned int visible_lights = 0;
	unsigned int visible_models = 0;
private:
	std::vector<std::shared_ptr<Renderable>> objects;

	std::map<std::string, std::shared_ptr<Camera::BaseCamera>> cameras;
	std::map<std::string, std::shared_ptr<InstancedQuad>> instanced_quads;
	std::map<std::string, std::shared_ptr<InstancedModel>> instanced_models;

	std::map<std::string, Shader> shaders;
	std::map<std::string, Texture> textures;


	unsigned int m_SimpleQuadVAO = 0;
	unsigned int m_SimpleQuadVBO;
};

#endif // !RENDERER_CLASS_DECL