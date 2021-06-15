#include <iostream>
#include <imgui/imgui.h>
#include "../Renderer.h"
#include "FlyCamera.h"

namespace Camera {
	FlyCamera::FlyCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
		: m_FrontVec(glm::vec3(0.0f, 0.0f, -1.0f)), BaseCamera()
	{
		m_CamType = Camera::Camera_Type::FLYCAM;
		m_Position = position;
		m_WorldUpVec = up;
		m_Yaw = yaw;
		m_Pitch = pitch;
		Update();
	}

	FlyCamera::FlyCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
		: m_FrontVec(glm::vec3(0.0f, 0.0f, -1.0f)), BaseCamera()
	{
		m_Position = glm::vec3(posX, posY, posZ);
		m_WorldUpVec = glm::vec3(upX, upY, upZ);
		m_Yaw = yaw;
		m_Pitch = pitch;
		Update();
	}

	glm::mat4 FlyCamera::GetViewMatrix()
	{
		return glm::lookAt(m_Position, m_Position + m_FrontVec, m_UpVec);
	}

	void FlyCamera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = m_MoveSpeed * deltaTime;
		if (direction == Camera_Movement::FORWARD)
			m_Position += m_FrontVec * velocity;
		if (direction == Camera_Movement::BACKWARD)
			m_Position -= m_FrontVec * velocity;
		if (direction == Camera_Movement::LEFT)
			m_Position -= m_RightVec * velocity;
		if (direction == Camera_Movement::RIGHT)
			m_Position += m_RightVec * velocity;
	}

	void FlyCamera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
	{
		xoffset *= m_MouseSensitivity;
		yoffset *= m_MouseSensitivity;

		m_Yaw += xoffset;
		m_Pitch += yoffset;

		if (constrainPitch)
		{
			if (m_Pitch > 89.0f)
				m_Pitch = 89.0f;
			if (m_Pitch < -89.0f)
				m_Pitch = -89.0f;
		}

		Update();
	}

	void FlyCamera::ProcessMouseScroll(float yoffset)
	{
		m_Zoom -= (float)yoffset;
		if (m_Zoom < 1.0f)
			m_Zoom = 1.0f;
		if (m_Zoom > 45.0f)
			m_Zoom = 45.0f;
	}

	void FlyCamera::Update()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		front.y = sin(glm::radians(m_Pitch));
		front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		m_FrontVec = glm::normalize(front);

		m_RightVec = glm::normalize(glm::cross(m_FrontVec, m_WorldUpVec));
		m_UpVec = glm::normalize(glm::cross(m_RightVec, m_FrontVec));
	}

	void FlyCamera::UI_Description() 
	{
		ImGui::TextUnformatted(U8_CAST("Тип: вільна"));
		ImGui::SliderFloat3(U8_CAST("Координати"), &m_Position.x, -100.f, 100.f);
		ImGui::DragFloat(U8_CAST("Масштабування"), &m_Zoom, 0.5, 0.5, 100.f);
	}
}