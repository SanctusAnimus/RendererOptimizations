#pragma once
#ifndef LIGHT_COMPONENT_DECL
#define LIGHT_COMPONENT_DECL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

struct LightComponent {
	float m_Constant = 1.0;

	glm::vec3 m_Color = glm::vec3(0.8f, 0.2, 0.3);
	float m_Linear = 0.7;
	float m_Quadratic = 2.1;
	float m_Radius = 0.f;
	float m_Intensity = 1.f;

	LightComponent() = default;
	LightComponent(const LightComponent&) = default;


	LightComponent(const glm::vec3& color) : m_Color(color) 
	{
		UpdateRadius();
	}
	LightComponent(const glm::vec3& color, float linear, float quadratic)
		: m_Color(color), m_Linear(linear), m_Quadratic(quadratic) 
	{
		UpdateRadius();
	}

	void SetIntensity(float intensity) {
		m_Intensity = intensity;
		UpdateRadius();
	}

	void UpdateRadius() {
		const float maxBrightness = std::fmaxf(std::fmaxf(m_Color.r, m_Color.g), m_Color.b);
		float radius = (-m_Linear + std::sqrt(m_Linear * m_Linear - 4 * m_Quadratic * (m_Constant - ((256.0f / 5.f) * maxBrightness)))) / (2.0f * m_Quadratic);
		m_Radius = m_Intensity * radius;
	}
};

#endif // !LIGHT_COMPONENT_DECL
