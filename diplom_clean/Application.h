#pragma once
#ifndef APPLICATION_CLASS_DECL
#define APPLICATION_CLASS_DECL

#include <memory>
#include "ImGui/ImGuiLayer.h"
#include "Scenes/BaseScene.h"
#include "Renderer/Renderer.h"

class Application
{
public:
	Application();

	void Init();
	void SetupCallbacks();
	void SetupScene();
	void Run();

	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
private:
	void processInput();

	std::shared_ptr<BaseScene> m_CurrentScene;

	GLFWwindow* m_Window;
	Renderer* m_Renderer;
	std::shared_ptr<ImGuiLayer> m_ImguiLayer;

	float m_DeltaTime = 0.0f;	// Time between current frame and last frame
	float m_LastFrame = 0.0f; // Time of last frame

	bool m_InitialMouseState = true;
	bool m_MouseHoldState = false;

	float m_MouseLastPosX;
	float m_MouseLastPosY;
};

struct WindowData {
	Application* app;
};
#endif // !APPLICATION_CLASS_DECL



