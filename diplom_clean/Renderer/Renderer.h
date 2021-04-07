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
		float light_constant = 1.0;
		float light_linear = 0.7;
		float light_quadratic = 2.1;
		float intensity = 1.0;

		float ambient = 0.1;
		float bloom_threshold = 1.0;
		int bloom_radius = 10.0;
		float exposure = 1.0;

		bool wireframe = false;
		bool skybox = false;
		
		float models_sphere_radius = 1.0;
	};
};

class Renderer
{
public:
	Renderer();

	// Model& NewModel();
	std::shared_ptr<Quad> NewQuad(std::string texture_name, std::string shader_name);
	std::shared_ptr<InstancedQuad> NewInstancedQuad(std::string texture_name, std::string shader_name, std::string instance_name);

	std::shared_ptr<InstancedModel> NewInstancedModel(std::string model_path, std::string shader_name, std::string instance_name);

	// basic VS + FS shader
	std::shared_ptr<Shader> NewShader(const char* vertex_path, const char* fragment_path, std::string name);
	// VS + FS + GS
	std::shared_ptr<Shader> NewShader(const char* vertex_path, const char* fragment_path, const char* geometry_pass, std::string name);
	// VS + FS + TCS + TES + GS
	std::shared_ptr<Shader> NewShader(const char* vertex_path, const char* fragment_path, const char* tess_control_path, const char* tess_eval_path, std::string name);

	std::shared_ptr<Texture> NewTexture(const char* file_path, std::string name, std::string type, const bool gamma = false);
	std::shared_ptr<Texture> NewCubemap(std::vector<std::string> faces);

	std::shared_ptr<Camera::BaseCamera> NewCamera(glm::vec3 coords, std::string camera_name, Camera::Camera_Type cam_type);

	std::shared_ptr<Shader> GetShader(std::string shader_name);
	std::shared_ptr<Texture> GetTexture(std::string texture_name);
	std::shared_ptr<InstancedQuad> GetInstancedQuad(std::string instance_name);

	static Renderer& instance()
	{
		static Renderer s_instance;
		return s_instance;
	};

	void Reset();

	void Render();
	void GatherImGui();

	void SimpleQuad();

	bool SetActiveCamera(std::string camera_name);

	std::shared_ptr<Camera::BaseCamera> m_CurrentCamera;
	Camera::Camera_Type m_CurrentCameraType;
	float m_FrameTime = 0.1f;
	Rendering::RenderSettings m_Settings;
	unsigned int m_VisibleLights = 0;
	unsigned int m_VisibleModels = 0;
private:
	std::vector<std::shared_ptr<Renderable>> m_Objects;

	std::map<std::string, std::shared_ptr<Camera::BaseCamera>> m_Cameras;
	std::map<std::string, std::shared_ptr<InstancedQuad>> m_InstancedQuad;
	std::map<std::string, std::shared_ptr<InstancedModel>> m_InstancedModels;

	std::map<std::string, std::shared_ptr<Shader>> m_Shaders;
	std::map<std::string, std::shared_ptr<Texture>> m_Textures;


	unsigned int m_SimpleQuadVAO = 0;
	unsigned int m_SimpleQuadVBO;
};

#endif // !RENDERER_CLASS_DECL