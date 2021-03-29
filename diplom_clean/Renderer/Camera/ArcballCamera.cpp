#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <imgui/imgui.h>

#include "ArcballCamera.h"


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

	}

	void ArcballCamera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
		glm::vec4 position(m_Position, 1);
		glm::vec4 pivot(m_lookAt, 1);

		// step 1 : Calculate the amount of rotation given the mouse movement.
		float deltaAngleX = (2 * M_PI / SCR_WIDTH); // a movement from left to right = 2*PI = 360 deg
		float deltaAngleY = (M_PI / SCR_HEIGHT);  // a movement from top to bottom = PI = 180 deg
		float xAngle = -xoffset * deltaAngleX;
		float yAngle = yoffset * deltaAngleY;

		// Extra step to handle the problem when the camera direction is the same as the up vector
		float cosAngle = glm::dot(GetViewDir(), m_upVector);
		if (cosAngle * sgn(deltaAngleY) > 0.99f)
			deltaAngleY = 0;

		// step 2: Rotate the camera around the pivot point on the first axis.
		glm::mat4x4 rotationMatrixX(1.0f);
		rotationMatrixX = glm::rotate(rotationMatrixX, xAngle, m_upVector);
		position = (rotationMatrixX * (position - pivot)) + pivot;

		// step 3: Rotate the camera around the pivot point on the second axis.
		glm::mat4x4 rotationMatrixY(1.0f);
		rotationMatrixY = glm::rotate(rotationMatrixY, yAngle, GetRightVector());
		glm::vec3 finalPosition = (rotationMatrixY * (position - pivot)) + pivot;

		// Update the camera view (we keep the same lookat and the same up vector)
		SetCameraView(finalPosition, m_lookAt, m_upVector);
	}

	void ArcballCamera::ProcessMouseScroll(float yoffset) {
		glm::vec3 direction_reversed = (yoffset / 2.0f) * glm::normalize(m_lookAt - m_Position);
		m_Position += direction_reversed;
		Update();
	}

	void ArcballCamera::Update() {
		/*
		std::cout << "updating view matrix" <<
			Position.x << " | " << Position.y << "|" << Position.z << "\n" <<
			m_lookAt.x << " | " << m_lookAt.y << "|" << m_lookAt.z << "\n" <<
			m_upVector.x << " | " << m_upVector.y << "|" << m_upVector.z << std::endl;
		*/
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
