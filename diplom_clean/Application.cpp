#include "Application.h"
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
#include <iostream>
#include <imgui/imgui.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Scenes/StartingScene.h"

Application::Application() {

}

void Application::Init() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    stbi_set_flip_vertically_on_load(true);

    m_Window = glfwCreateWindow(Rendering::SCREEN_WIDTH, Rendering::SCREEN_HEIGHT, "Merenkov D.M. Diplom", NULL, NULL); // glfwGetPrimaryMonitor()
    if (m_Window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(m_Window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    m_Renderer = &Renderer::instance();
    imgui_layer = std::shared_ptr<ImGuiLayer>(new ImGuiLayer(m_Window, "core_layer"));

    lastX = Rendering::SCREEN_WIDTH / 2.0;
    lastY = Rendering::SCREEN_HEIGHT / 2.0;

    WindowData window_data;
    window_data.app = this;

    glfwSetWindowUserPointer(m_Window, &window_data);

    this->SetupCallbacks();
    this->SetupScene();
    std::cout << "starting loop" << std::endl;


	while(!glfwWindowShouldClose(m_Window)) {
		this->Run();
	}

    glfwTerminate();
    return;
}

void Application::SetupCallbacks() {
    std::cout << "Setuping callbacks..." << std::endl;
    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        WindowData& window_data = *(WindowData*)glfwGetWindowUserPointer(window);
        window_data.app->framebuffer_size_callback(window, width, height);
    });
    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
        WindowData& window_data = *(WindowData*)glfwGetWindowUserPointer(window);
        window_data.app->mouse_button_callback(window, button, action, mods);
    });
    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
        WindowData& window_data = *(WindowData*)glfwGetWindowUserPointer(window);
        window_data.app->mouse_callback(window, xpos, ypos);
    });
    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset) {
        WindowData& window_data = *(WindowData*)glfwGetWindowUserPointer(window);
        window_data.app->scroll_callback(window, xoffset, yoffset);
    });
    std::cout << "...Done" << std::endl;
}

void Application::SetupScene() {
    std::cout << "Setuping scene..." << std::endl;
    current_scene = std::shared_ptr<BaseScene>(new StartingScene());
    current_scene->Setup();
    std::cout << "...Done" << std::endl;
}

void Application::Run() {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    m_Renderer->frame_time = deltaTime;

    processInput();

    imgui_layer->Begin();

    current_scene->Render();

    imgui_layer->End(Rendering::SCREEN_WIDTH, Rendering::SCREEN_HEIGHT);

    glfwSwapBuffers(m_Window);
    glfwPollEvents();
}



void Application::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Rendering::SCREEN_WIDTH = width;
    Rendering::SCREEN_HEIGHT = height;
    glViewport(0, 0, width, height);
}

void Application::processInput()
{
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_Window, true);

    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
        m_Renderer->current_camera->ProcessKeyboard(Camera::FORWARD, m_Renderer->frame_time);
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
        m_Renderer->current_camera->ProcessKeyboard(Camera::BACKWARD, m_Renderer->frame_time);
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
        m_Renderer->current_camera->ProcessKeyboard(Camera::LEFT, m_Renderer->frame_time);
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
        m_Renderer->current_camera->ProcessKeyboard(Camera::RIGHT, m_Renderer->frame_time);

    if (glfwGetKey(m_Window, GLFW_KEY_3) == GLFW_PRESS) {
        if (m_Renderer->current_camera_type != Camera::Camera_Type::ARCBALL) {
            std::cout << "switching to arcball camera" << std::endl;
            glm::vec3 position = m_Renderer->current_camera->m_Position;
            m_Renderer->SetActiveCamera("arcball_camera");
            m_Renderer->current_camera->m_Position = position;
            m_Renderer->current_camera->Update();
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    if (glfwGetKey(m_Window, GLFW_KEY_4) == GLFW_PRESS) {
        if (m_Renderer->current_camera_type != Camera::Camera_Type::FLYCAM) {
            std::cout << "switching to fly camera" << std::endl;
            glm::vec3 position = m_Renderer->current_camera->m_Position;
            m_Renderer->SetActiveCamera("fly_camera");
            m_Renderer->current_camera->m_Position = position;
            m_Renderer->current_camera->Update();
            glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

void Application::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if ((mouse_hold
        || Renderer::instance().current_camera_type == Camera::Camera_Type::FLYCAM)
        && !ImGui::IsAnyItemActive()) {
        Renderer::instance().current_camera->ProcessMouseMovement(xoffset, yoffset);
    }
}

void Application::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) mouse_hold = true;
        if (action == GLFW_RELEASE) mouse_hold = false;
    }
}

void Application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (ImGui::IsAnyWindowHovered()) return;
    Renderer::instance().current_camera->ProcessMouseScroll(yoffset);
}