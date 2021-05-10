#pragma once
#ifndef RENDERER_CLASS_DECL
#define RENDERER_CLASS_DECL

#include <vector>
#include <map>
#include <entt/entt.hpp>

#include "Texture.h"
#include "Model/Model.h"
#include "Model/InstancedModel.h"
#include "Quad/Quad.h"
#include "Quad/InstancedQuad.h"
#include "Camera/BaseCamera.h"
#include "Camera/FlyCamera.h"
#include "Camera/ArcballCamera.h"

#define U8_CAST(string) reinterpret_cast<const char*>(u8##string)

namespace Rendering {
	extern unsigned int SCREEN_WIDTH;
	extern unsigned int SCREEN_HEIGHT;

	struct RenderSettings {
		float light_constant = 1.0f;
		float light_linear = 0.07f;
		float light_quadratic = 0.21f;
		float light_spread = 10.f;
		float intensity = 1.5f;

		float ambient = 0.1f;
		float bloom_threshold = 1.0f;
		int bloom_radius = 10;
		int current_light_limits = 16;
		int current_model_limits = 16;
		float exposure = 1.0f;

		bool wireframe = false;
		bool skybox = false;
		
		float models_sphere_radius = 1.0f;

		bool _compress_textures = false;
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

	std::shared_ptr<Texture> NewTexture(const char* file_path, std::string name, std::string type, const bool compress = true);
	std::shared_ptr<Texture> NewCubemap(std::vector<std::string> faces);

	std::shared_ptr<Camera::BaseCamera> NewCamera(glm::vec3 coords, std::string camera_name, Camera::Camera_Type cam_type);

	std::shared_ptr<Shader> GetShader(std::string shader_name);
	std::shared_ptr<Texture> GetTexture(std::string texture_name);
	std::shared_ptr<InstancedQuad> GetInstancedQuad(std::string instance_name);
	std::shared_ptr<InstancedModel> GetInstancedModel(std::string instance_name);

	static Renderer& instance()
	{
		static Renderer s_instance;
		return s_instance;
	};

	void Reset();

	void Render(entt::registry& registry);
	void GatherImGui();

	void PerfCounter();  // separate to render inside main window
	void Dockspace();

	void SimpleQuad();

	bool SetActiveCamera(std::string camera_name);

	std::shared_ptr<Camera::BaseCamera> m_CurrentCamera;
	Camera::Camera_Type m_CurrentCameraType;
	float m_FrameTime = 0.1f;
	Rendering::RenderSettings m_Settings;
	size_t m_VisibleLights = 0;
	size_t m_VisibleModels = 0;
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