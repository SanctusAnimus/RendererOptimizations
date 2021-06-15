#include "Application.h"
#include <iostream>
#include <unordered_map>
#include <imgui/imgui.h>
#include <time.h>
#include <chrono>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Scenes/SceneIndex.h"

#include "ImGui/ImGuiLogger.h"

OptimizationsApp::OptimizationsApp() {

}

void OptimizationsApp::Init() {
    // встановлення версії специфікації OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    stbi_set_flip_vertically_on_load(true);

    // отримання параметрів монітора, та присвоєння їх до створенного вікна
    auto monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    Rendering::SCREEN_WIDTH = 1920;
    Rendering::SCREEN_HEIGHT = 1060;

    // ініціалізація вікна програми
    m_Window = glfwCreateWindow(Rendering::SCREEN_WIDTH, Rendering::SCREEN_HEIGHT, "Merenkov D.M. Bachelors", NULL, NULL);
    if (m_Window == NULL)
    {
        // вихід з програми, якщо вікно не було створено
        std::cout << "[OptimizationsApp] Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    // завантаження специфікації
    glfwMakeContextCurrent(m_Window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "[OptimizationsApp] Failed to initialize GLAD" << std::endl;
        return;
    }
    // налаштування парметрів специфікаії
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // створення візуалізатора та обробника графічного інтерфейсу
    m_Renderer = &Renderer::instance();
    m_ImguiLayer = std::shared_ptr<ImGuiLayer>(new ImGuiLayer(m_Window, "core_layer"));

    m_MouseLastPosX = Rendering::SCREEN_WIDTH / 2.0f;
    m_MouseLastPosY = Rendering::SCREEN_HEIGHT / 2.0f;

    WindowData window_data;
    window_data.app = this;

    glfwSetWindowUserPointer(m_Window, &window_data);

    // налаштування обробника подій та стартової сцени
    this->SetupCallbacks();
    this->SetupScene();
    
    // головний цикл програми
    std::cout << "[OptimizationsApp] starting main loop" << std::endl;
	while(!glfwWindowShouldClose(m_Window)) {
		this->Run();
	}
    std::cout << "[OptimizationsApp] closed main loop" << std::endl;
    // завершення роботи програми
    glfwTerminate();
    return;
}

void OptimizationsApp::SetupCallbacks() {
    Logger::instance().AddLog("[OptimizationsApp] Callbacks setup started...\n");
    // встановлення зворотних викликів для обробки зміни розміру екрану, переміщення курсора миші, натискання кнопок миші
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

    Logger::instance().AddLog("[OptimizationsApp] Callbacks setup finished.\n");
}

void OptimizationsApp::SetupScene() {
    // створення та налаштування стартової сцени додатка
    Logger::instance().AddLog("[OptimizationsApp] Scene setup started...\n");
    m_CurrentScene = std::shared_ptr<BaseScene>(new RawScene());
    m_CurrentScene->Setup();
    Logger::instance().AddLog("[OptimizationsApp] Scene setup finished.\n");
}

void OptimizationsApp::Run() {
    // обрахування витраченого часу на візуалізацію останнього кадру
    Logger* logger = &Logger::instance();
    float currentFrame = static_cast<float>(glfwGetTime());
    m_DeltaTime = currentFrame - m_LastFrame;
    m_LastFrame = currentFrame;
    m_Renderer->m_FrameTime = m_DeltaTime;

    // прослуховування та обробка подій вікна
    glfwPollEvents();
    processInput();

    // візуалізація активної сцени
    m_ImguiLayer->Begin();

    m_CurrentScene->Render();

    // візуалзіація вікна логгеру
    logger->Draw(U8_CAST("Логгер"));

    // візуалізація вікна вибору сцени
    if (ImGui::Begin(U8_CAST("Вибір сцени"))) {
        // список сцен на зворотні виклики для створення нових активних сцен за обраною назвою
        using options_map = std::map<std::string, std::function<void()>>;
        static options_map options = {
            {U8_CAST("1. Базова сцена"), [this]() {
                m_CurrentScene = std::shared_ptr<BaseScene>(new RawScene());
                m_CurrentScene->Setup();
            }},
            {U8_CAST("2. Дублювання геометрії"), [this]() {
                m_CurrentScene = std::shared_ptr<BaseScene>(new InstancingScene());
                m_CurrentScene->Setup();
            }},
            {U8_CAST("3. Відкладене затінення"), [this]() {
                m_CurrentScene = std::shared_ptr<BaseScene>(new DeferredScene());
                m_CurrentScene->Setup();
            }},
            {U8_CAST("4. Обрізка фрустумом"), [this]() {
                m_CurrentScene = std::shared_ptr<BaseScene>(new FrustumScene());
                m_CurrentScene->Setup();
            }},
            {U8_CAST("5. Стиснення текстур"), [this]() {
                m_CurrentScene = std::shared_ptr<BaseScene>(new StartingScene());
                m_CurrentScene->Setup();
            }}
        };
        static int item_current_idx = 0;
        static std::string current_key = U8_CAST("1. Базова сцена");
        if (ImGui::BeginCombo(U8_CAST("Обрана сцена"), current_key.c_str())) {
            int n = 0;
            for (auto& [key, callback] : options) {
                const bool is_selected = (item_current_idx == n);
                if (ImGui::Selectable(key.c_str(), is_selected)) {
                    item_current_idx = n;
                    current_key = key;
                    Renderer::instance().Reset();
                    callback();
                }
                
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
                n++;
            }
            ImGui::EndCombo();
        }
    }
    ImGui::End();
 
    // перемикання буферу на відмальований кадр
    m_ImguiLayer->End(Rendering::SCREEN_WIDTH, Rendering::SCREEN_HEIGHT);
    glfwSwapBuffers(m_Window);
}



void OptimizationsApp::framebuffer_size_callback(GLFWwindow* window, int width, int height) {}

void OptimizationsApp::processInput()
{
    // обробка кнопки ESCAPE для закриття додатку з клавіатури
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_Window, true);

    // обробка кнопок переміщення камери - W, A, S, D
    if (glfwGetKey(m_Window, GLFW_KEY_W) == GLFW_PRESS)
        m_Renderer->m_CurrentCamera->ProcessKeyboard(Camera::Camera_Movement::FORWARD, m_Renderer->m_FrameTime);
    if (glfwGetKey(m_Window, GLFW_KEY_S) == GLFW_PRESS)
        m_Renderer->m_CurrentCamera->ProcessKeyboard(Camera::Camera_Movement::BACKWARD, m_Renderer->m_FrameTime);
    if (glfwGetKey(m_Window, GLFW_KEY_A) == GLFW_PRESS)
        m_Renderer->m_CurrentCamera->ProcessKeyboard(Camera::Camera_Movement::LEFT, m_Renderer->m_FrameTime);
    if (glfwGetKey(m_Window, GLFW_KEY_D) == GLFW_PRESS)
        m_Renderer->m_CurrentCamera->ProcessKeyboard(Camera::Camera_Movement::RIGHT, m_Renderer->m_FrameTime);

    if (!m_CurrentScene->m_Active) return;
    // обробка кнопок переключення типів камер - 3 та 4
    if (glfwGetKey(m_Window, GLFW_KEY_3) == GLFW_PRESS) {
        if (m_Renderer->m_CurrentCameraType != Camera::Camera_Type::ARCBALL) {
            glm::vec3 position = m_Renderer->m_CurrentCamera->m_Position;
            if (m_Renderer->SetActiveCamera("arcball_camera")) {
                m_Renderer->m_CurrentCamera->m_Position = position;
                m_Renderer->m_CurrentCamera->Update();
                glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

                Logger::instance().AddLog("[Input] Switching camera to arcball type\n");
            }
        }
    }
    if (glfwGetKey(m_Window, GLFW_KEY_4) == GLFW_PRESS) {
        if (m_Renderer->m_CurrentCameraType != Camera::Camera_Type::FLYCAM) {
            glm::vec3 position = m_Renderer->m_CurrentCamera->m_Position;
            if (m_Renderer->SetActiveCamera("fly_camera")) {
                m_Renderer->m_CurrentCamera->m_Position = position;
                m_Renderer->m_CurrentCamera->Update();
                glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                Logger::instance().AddLog("[Input] Switching camera to flying type\n");
            }
        }
    }
}

void OptimizationsApp::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // ініціалізація параметрів вказівника миші при першому виклику
    if (m_InitialMouseState)
    {
        m_MouseLastPosX = static_cast<float>(xpos);
        m_MouseLastPosY = static_cast<float>(ypos);
        m_InitialMouseState = false;
    }

    // обрахування відносного переміщення вказівника
    float xoffset = static_cast<float>(xpos) - m_MouseLastPosX;
    float yoffset = m_MouseLastPosY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

    m_MouseLastPosX = static_cast<float>(xpos);
    m_MouseLastPosY = static_cast<float>(ypos);

    // обробка переміщення для активної камери
    if (
        (m_MouseHoldState && m_CurrentScene->m_Active)
        || Renderer::instance().m_CurrentCameraType == Camera::Camera_Type::FLYCAM)
    {
        Renderer::instance().m_CurrentCamera->ProcessMouseMovement(xoffset, yoffset);
    }
}

void OptimizationsApp::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    // обробка натискання лівої кнопки миші
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) m_MouseHoldState = true;
        if (action == GLFW_RELEASE) m_MouseHoldState = false;
    }
}

void OptimizationsApp::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // обробка прокрутки колеса миші
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += static_cast<float>(xoffset);
    io.MouseWheel += static_cast<float>(yoffset);

    if (!m_CurrentScene->m_Active) return;
    Renderer::instance().m_CurrentCamera->ProcessMouseScroll(static_cast<float>(yoffset));
}