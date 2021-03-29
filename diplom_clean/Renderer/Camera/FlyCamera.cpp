#include <iostream>
#include <imgui/imgui.h>

#include "FlyCamera.h"

namespace Camera {
	FlyCamera::FlyCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
		: Front(glm::vec3(0.0f, 0.0f, -1.0f)), BaseCamera()
	{
		m_CamType = Camera::Camera_Type::FLYCAM;
		m_Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		Update();
	}

	FlyCamera::FlyCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
		: Front(glm::vec3(0.0f, 0.0f, -1.0f)), BaseCamera()
	{
		m_Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		Update();
	}

	glm::mat4 FlyCamera::GetViewMatrix()
	{
		return glm::lookAt(m_Position, m_Position + Front, Up);
	}

	void FlyCamera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = m_MoveSpeed * deltaTime;
		if (direction == FORWARD)
			m_Position += Front * velocity;
		if (direction == BACKWARD)
			m_Position -= Front * velocity;
		if (direction == LEFT)
			m_Position -= Right * velocity;
		if (direction == RIGHT)
			m_Position += Right * velocity;
	}

	void FlyCamera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
	{
		xoffset *= m_MouseSensitivity;
		yoffset *= m_MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
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
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);

		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
	}

	void FlyCamera::UI_Description() 
	{
		ImGui::TextUnformatted("Type: Flying");
		ImGui::SliderFloat3("Position", &m_Position.x, -100.f, 100.f);
		ImGui::DragFloat("Zoom:", &m_Zoom, 0.5, 0.5, 100.f);
	}
}