#include "ImGuiLayer.h"
#include <iostream>
#include "imgui/imgui.h"

#include "imgui/examples/imgui_impl_opengl3.h"
#include "imgui/examples/imgui_impl_glfw.h"


ImGuiLayer::ImGuiLayer(GLFWwindow* window, std::string name) : name(name)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 420");
	ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.TTF", 14.0F, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	std::cout << "ImGui Layer created " << name << std::endl;
}

ImGuiLayer::~ImGuiLayer()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	std::cout << "ImGui Layer destroyed " << name << std::endl;
}

void ImGuiLayer::Begin()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}
/*
std::tuple<bool, bool, bool, bool, bool> ImGuiLayer::Run(float width, float height)
{
	static bool show = true;
	bool plot_param_changed = false;
	bool translation_changed = false;
	bool rotation_changed = false;
	bool affine_changed = false;
	bool projection_changed = false;

	//ImGui::ShowDemoWindow(&show);
	if (!ImGui::Begin(u8"Графика", &show))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return { plot_param_changed, translation_changed, rotation_changed, affine_changed, projection_changed };
	}
	ImGui::Text(u8"Размер клетки: 25 пикселей");
	if (ImGui::CollapsingHeader(u8"Параметры", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Checkbox(u8"Коллайдеры", &this->draw_bounding_boxes);
		if (ImGui::Checkbox(u8"Контрольные точки", &this->enable_controls)) {
			plot_param_changed = true;
		}

		ImGui::InputInt(u8"Количество точек", &this->points_count, 100, 1000);
		if (ImGui::Button(u8"Применить")) {
			plot_param_changed = true;
		}

		if (ImGui::Button("Save")) {
			std::cout << "Clicked save!" << std::endl;
			this->save_file = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Load")) {
			std::cout << "Clicked load!" << std::endl;
			this->load_file = true;
			this->enable_controls = true;
		}
	}

	if (ImGui::CollapsingHeader(u8"Эвклидовые преобразования", &show, ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::InputInt2(u8"Величины сдвига", this->tranlation, 1);
		if (ImGui::Button(u8"Сдвинуть")) {
			std::cout << "Translation changed" << std::endl;
			translation_changed = true;
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"Сбросить сдвиг")) {
			std::cout << "Translation reset" << std::endl;
			translation_changed = true;
			this->reset = true;
		}
		ImGui::Separator();
		ImGui::InputInt2(u8"Точка поворота", this->rotation_point, 1);
		ImGui::InputInt(u8"Угол", &this->rotation_angle, 1);
		if (ImGui::Button(u8"Повернуть")) {
			std::cout << "Rotation changed" << std::endl;
			rotation_changed = true;
		}
		ImGui::SameLine();
		if (ImGui::Button(u8"Сбросить поворот")) {
			this->rotation_point[0] = 0;
			this->rotation_point[1] = 0;
			this->rotation_angle = 0;
			std::cout << "Rotation reset" << std::endl;
			rotation_changed = true;
			this->reset = true;
		}
	}

	ImGui::End();

	if (!ImGui::Begin(u8"Аффинные")) {
		ImGui::End();
		return { plot_param_changed, translation_changed, rotation_changed, affine_changed, projection_changed };
	}
	ImGui::InputFloat2(u8"r0", this->affine_r0, 1);
	ImGui::InputFloat2(u8"rX", this->affine_rX, 1);
	ImGui::InputFloat2(u8"rY", this->affine_rY, 1);
	if (ImGui::Button(u8"Преобразовать")) {
		std::cout << "Affine transform used" << std::endl;
		affine_changed = true;
	}
	ImGui::SameLine();
	if (ImGui::Button(u8"Сбросить аффинные")) {
		std::cout << "Affine transform reset" << std::endl;
		this->reset = true;
		affine_changed = true;
	}
	ImGui::End();

	if (!ImGui::Begin(u8"Проективные")) {
		ImGui::End();
		return { plot_param_changed, translation_changed, rotation_changed, affine_changed, projection_changed };
	}
	ImGui::InputFloat3(u8"r0", this->proj_r0, 1);
	ImGui::InputFloat3(u8"rX", this->proj_rX, 1);
	ImGui::InputFloat3(u8"rY", this->proj_rY, 1);
	if (ImGui::Button(u8"Преобразовать")) {
		std::cout << "Proj transform used" << std::endl;
		projection_changed = true;
	}
	ImGui::SameLine();
	if (ImGui::Button(u8"Сбросить проективные")) {
		std::cout << "Proj transform reset" << std::endl;
		this->reset = true;
		projection_changed = true;
	}
	ImGui::End();
	//std::cout << "ImGuiLayer finished run, results: " << plot_param_changed << " " << translation_changed << " " << rotation_changed << std::endl;
	return { plot_param_changed, translation_changed, rotation_changed, affine_changed, projection_changed };
}
*/
void ImGuiLayer::End(float width, float height)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(width, height);
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}
