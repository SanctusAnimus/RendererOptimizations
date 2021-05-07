#pragma once
#ifndef BASESCENE_CLASS_DECL
#define BASESCENE_CLASS_DECL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <entt/entt.hpp>

#include "../Renderer/Model/Model.h"
#include "../Renderer/Model/InstancedModel.h"
#include "../Renderer/LightData.h"
#include "../Renderer/LightSource.h"
#include "../Renderer/Buffers/GLTexture.h"
#include "../Renderer/Quad/Quad.h"

bool IsVisibleFrom(glm::vec4 position, glm::mat4& mvp, float radius);
std::vector<float> GetSkyboxVerts();

class BaseScene
{
public:
	virtual void Setup() = 0;
	virtual void Render() = 0;

	virtual entt::entity AddEntity() {
		return m_Registry.create();
	}
	virtual void RemoveEntity(entt::entity& ent) {
		m_Registry.destroy(ent);
	}

	void AddInstancedModel(entt::registry& registry, entt::entity entity);
	void UpdateInstancedModel(entt::registry& registry, entt::entity entity);

	void BuildLightData();
	void RegenerateLights(std::vector<Rendering::LightData>& visible_lights);

	bool m_Active = false;
protected:
	entt::registry m_Registry;

	float spread = 20.f;
	const int LIGHT_COUNT_MAX = 128;

	int m_ModelsStride = 13;
};

#endif // !BASESCENE_CLASS_DECL



