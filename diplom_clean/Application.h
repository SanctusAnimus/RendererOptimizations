#pragma once
#ifndef APPLICATION_CLASS_DECL
#define APPLICATION_CLASS_DECL
// #include <GLFW/glfw3.h>
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

	std::shared_ptr<BaseScene> current_scene;

	GLFWwindow* m_Window;
	Renderer* m_Renderer;
	std::shared_ptr<ImGuiLayer> imgui_layer;

	float deltaTime = 0.0f;	// Time between current frame and last frame
	float lastFrame = 0.0f; // Time of last frame

	bool firstMouse = true;
	bool mouse_hold = false;

	float lastX;
	float lastY;
};

struct WindowData {
	Application* app;
};
#endif // !APPLICATION_CLASS_DECL



