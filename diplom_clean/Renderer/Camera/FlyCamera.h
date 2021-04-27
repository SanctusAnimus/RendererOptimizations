#pragma once
#ifndef CAMERA_CLASS_DECL
#define CAMERA_CLASS_DECL

#include "BaseCamera.h"

namespace Camera {
    class FlyCamera : public BaseCamera
    {
    public:
        // camera Attributes
        glm::vec3 m_FrontVec;
        glm::vec3 m_UpVec;
        glm::vec3 m_RightVec;
        glm::vec3 m_WorldUpVec;
        // euler Angles
        float m_Yaw;
        float m_Pitch;

        FlyCamera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
        FlyCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);
        glm::mat4 GetViewMatrix() override;
        void ProcessKeyboard(Camera_Movement direction, float deltaTime) override;
        void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) override;
        void ProcessMouseScroll(float yoffset) override;
        void UI_Description() override;
        void Update() override;
    private:
    };


}




#endif // !CAMERA_CLASS_DECL