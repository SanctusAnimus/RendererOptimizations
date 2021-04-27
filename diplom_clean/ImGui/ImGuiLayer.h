#pragma once
#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <tuple>
#include <imgui/imgui.h>
#include <string>


class ImGuiLayer {
public:
	ImGuiLayer() {};
	ImGuiLayer(GLFWwindow* window, std::string name);
	~ImGuiLayer();

	void Begin();
	void End(unsigned int width, unsigned int height);
private:
	std::string name;
};