#pragma once
#ifndef BASECAMERA_CLASS_DECL
#define BASECAMERA_CLASS_DECL

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Camera {
    enum Camera_Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    enum Camera_Type {
        FLYCAM,
        ARCBALL
    };

    // Default camera values
    const float YAW = -90.0f;
    const float PITCH = 0.0f;
    const float SPEED = 2.5f;
    const float SENSITIVITY = 0.1f;
    const float ZOOM = 45.0f;

    const unsigned int SCR_WIDTH = 1920; // 1920;
    const unsigned int SCR_HEIGHT = 1060; // 1060;

	class BaseCamera
	{
    public:
        glm::vec3 m_Position;

        // camera options
        float m_MoveSpeed;
        float m_MouseSensitivity;
        float m_Zoom;
        Camera::Camera_Type m_CamType;

        BaseCamera() : m_MoveSpeed(SPEED), m_MouseSensitivity(SENSITIVITY), m_Zoom(ZOOM) {};

        virtual glm::mat4 GetViewMatrix() { return glm::mat4(1.f); };
        virtual void ProcessKeyboard(Camera_Movement direction, float deltaTime) {};
        virtual void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {};
        virtual void ProcessMouseScroll(float yoffset) {};
        virtual void UI_Description() {};
        virtual void Update() {};
	};
}
#endif // !BASECAMERA_CLASS_DECL



