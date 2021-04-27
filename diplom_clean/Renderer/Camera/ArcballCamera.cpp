#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <imgui/imgui.h>

#include "ArcballCamera.h"
#include "../Renderer.h"


namespace Camera {
	template <typename T> int sgn(T val) {
		return (T(0) < val) - (val < T(0));
	}

	ArcballCamera::ArcballCamera(glm::vec3 coords)
		: BaseCamera(), m_lookAt(glm::vec3(0.0f, 0.0f, 0.f)), m_upVector(glm::vec3(0.f, 1.f, 0.f))
	{
		m_CamType = Camera::Camera_Type::ARCBALL;
		m_Position = glm::vec3(1.f, 1.f, 1.f);
		Update();
	};

	glm::mat4 ArcballCamera::GetViewMatrix() {
		return m_viewMatrix;
	}

	void ArcballCamera::SetCameraView(glm::vec3 position, glm::vec3 lookat, glm::vec3 up) {
		m_Position = std::move(position);
		m_lookAt = std::move(lookat);
		m_upVector = std::move(up);
		Update();
	}

	void ArcballCamera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
		float velocity = m_MoveSpeed * deltaTime;
		if (direction == Camera_Movement::FORWARD)
			m_lookAt.z += velocity;
		if (direction == Camera_Movement::BACKWARD)
			m_lookAt.z -= velocity;
		if (direction == Camera_Movement::LEFT)
			m_lookAt.x -= velocity;
		if (direction == Camera_Movement::RIGHT)
			m_lookAt.x += velocity;
		Update();
	}

	void ArcballCamera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
		glm::vec4 position(m_Position, 1);
		glm::vec4 pivot(m_lookAt, 1);

		float delta_angle_x = (2.f * (float)M_PI / (float)Rendering::SCREEN_WIDTH);
		float delta_angle_y = ((float)M_PI / (float)Rendering::SCREEN_HEIGHT);
		float angle_x = -xoffset * delta_angle_x;
		float angle_y = yoffset * delta_angle_y;

		float cosAngle = glm::dot(GetViewDir(), m_upVector);
		if (cosAngle * sgn(delta_angle_y) > 0.99f)
			delta_angle_y = 0;

		glm::mat4x4 rotation_mat_x(1.0f);
		rotation_mat_x = glm::rotate(rotation_mat_x, angle_x, m_upVector);
		position = (rotation_mat_x * (position - pivot)) + pivot;

		glm::mat4x4 rotation_mat_y(1.0f);
		rotation_mat_y = glm::rotate(rotation_mat_y, angle_y, GetRightVector());
		glm::vec3 final_position = (rotation_mat_y * (position - pivot)) + pivot;

		SetCameraView(final_position, m_lookAt, m_upVector);
	}

	void ArcballCamera::ProcessMouseScroll(float yoffset) {
		glm::vec3 direction_reversed = (yoffset / 2.0f) * glm::normalize(m_lookAt - m_Position);
		m_Position += direction_reversed;
		Update();
	}

	void ArcballCamera::Update() {
		m_viewMatrix = glm::lookAt(m_Position, m_lookAt, m_upVector);
	}

	void ArcballCamera::UI_Description() {
		static bool ac_changed = false;
		ImGui::TextUnformatted("Type: Arcball");
		if (ImGui::SliderFloat3("Position", &m_Position.x, -100.f, 100.f)) ac_changed = true;
		if (ImGui::DragFloat("Zoom:", &m_Zoom, 0.5, 0.5, 100.f)) ac_changed = true;
		if (ac_changed) Update();
	}
}
