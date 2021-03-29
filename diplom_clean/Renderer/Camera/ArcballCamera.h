#pragma once
#ifndef ARCBALL_CAMERA_CLASS_DECL
#define ARCBALL_CAMERA_CLASS_DECL

#include "BaseCamera.h"

namespace Camera {
	class ArcballCamera : public BaseCamera
	{
	public:
		ArcballCamera(glm::vec3 coords);

		glm::mat4 GetViewMatrix() override;

		void SetCameraView(glm::vec3 eye, glm::vec3 lookat, glm::vec3 up);

		void ProcessKeyboard(Camera_Movement direction, float deltaTime) override;
		void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) override;
		void ProcessMouseScroll(float yoffset) override;
		void UI_Description() override;
	private:
		glm::mat4 m_viewMatrix;
		glm::vec3 m_lookAt;
		glm::vec3 m_upVector;

		void Update() override;

		glm::vec3 GetViewDir() const { return -glm::transpose(m_viewMatrix)[2]; }
		glm::vec3 GetRightVector() const { return glm::transpose(m_viewMatrix)[0]; }
		glm::mat4 GetViewMatrix() const { return m_viewMatrix; }
		inline glm::vec3 GetPosition() const { return m_Position; }
		inline glm::vec3 GetUpVector() const { return m_upVector; }
		inline glm::vec3 GetLookAt() const { return m_lookAt; }
	};
}


#endif // !ARCBALL_CAMERA_CLASS_DECL



