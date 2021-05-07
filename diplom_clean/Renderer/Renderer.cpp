#include "Renderer.h"
#include "../stb_image.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include "../ImGui/ImGuiLogger.h"

#include "../Components/Render.h"
#include "../Components/Transform.h"

std::tuple<unsigned int, unsigned int> m_loadTexture(char const* path, const bool flip = true, const bool compress = true);
static bool open = true;

namespace Rendering {
    unsigned int SCREEN_WIDTH = 1920;
    unsigned int SCREEN_HEIGHT = 1060;
}

Renderer::Renderer() {
    NewTexture(
        "Resource/textures/missing.jpg", "missing", "texture_diffuse"
    );
    std::cout << "Renderer created" << std::endl;
}

static void GLClearErrors() {
    while (glGetError() != GL_NO_ERROR);
}

static void GLGetErrors() {
    while (GLenum error = glGetError()) {

        const char* description;
        int code = glfwGetError(&description);
        if (description != nullptr) {
            std::cout << "[OpenGL] error " << error << " " << description << std::endl;
        }
        else {
            std::cout << "[OpenGL] error " << error << " with empty message" << std::endl;
        }

    }
}

std::shared_ptr<Quad> Renderer::NewQuad(std::string texture_name, std::string shader_name) {
    Logger::instance().AddLog("[Renderer] New quad created.\n");

	auto shader = this->GetShader(shader_name);
	auto texture = this->GetTexture(texture_name);

    auto quad = std::shared_ptr<Quad>(new Quad(texture, shader));
    m_Objects.push_back(quad);

	return quad;
}

std::shared_ptr<InstancedQuad> Renderer::NewInstancedQuad(std::string texture_name, std::string shader_name, std::string instance_name) {
    Logger::instance().AddLog("[Renderer] New instanced quad with name %s\n", instance_name.c_str());

    auto shader = this->GetShader(shader_name);
    auto texture = this->GetTexture(texture_name);

    auto instance = std::shared_ptr<InstancedQuad>(new InstancedQuad(texture, shader));
    auto ret = m_InstancedQuad.insert({instance_name, instance});
    return instance;
}


std::shared_ptr<InstancedModel> Renderer::NewInstancedModel(std::string model_path, std::string shader_name, std::string instance_name) {
    Logger::instance().AddLog("[Renderer] New instanced model with name %s\n", instance_name.c_str());

    auto shader = this->GetShader(shader_name);

    auto instance = std::shared_ptr<InstancedModel>(new InstancedModel(model_path.c_str(), shader, m_Settings._compress_textures));
    auto ret = m_InstancedModels.insert({ instance_name, instance });
    return instance;
}

std::shared_ptr<Shader> Renderer::NewShader(const char* vertex_path, const char* fragment_path, std::string name) {
    Logger::instance().AddLog("[Renderer] New VS+FS shader with name %s\n", name.c_str());

    auto shader = std::shared_ptr<Shader>(new Shader(vertex_path, fragment_path, name));
	m_Shaders.insert({ name, shader });
	return shader;
}

std::shared_ptr<Shader> Renderer::NewShader(const char* vertex_path, const char* fragment_path, const char* geometry_pass, std::string name) {
    Logger::instance().AddLog("[Renderer] New VS+FS+GS shader with name %s\n", name.c_str());

    auto shader = std::shared_ptr<Shader>(new Shader(vertex_path, fragment_path, geometry_pass, name));
    m_Shaders.insert({ name, shader });
    return shader;
}


std::shared_ptr<Shader> Renderer::NewShader(const char* vertex_path, const char* fragment_path, const char* tess_control_path, const char* tess_eval_path, std::string name) {
    Logger::instance().AddLog("[Renderer] New VS+FS+TCS+TES+GS shader with name %s\n", name.c_str());

    auto shader = std::shared_ptr<Shader>(new Shader(vertex_path, fragment_path, tess_control_path, tess_eval_path, name));
    m_Shaders.insert({ name, shader });
    return shader;
}

std::shared_ptr<InstancedModel> Renderer::GetInstancedModel(std::string instance_name) {
    auto inst_ctrl = m_InstancedModels.find(instance_name);
    if (inst_ctrl != m_InstancedModels.end()) {
        return inst_ctrl->second;
    }
    return nullptr;
}


void Renderer::Render(entt::registry& registry) {
    glm::mat4 projection = glm::perspective(
        glm::radians(m_CurrentCamera->m_Zoom), 
        (float)Rendering::SCREEN_WIDTH / (float)Rendering::SCREEN_HEIGHT, 0.1f, 100.0f);

	for (auto& obj : m_Objects) {
		obj->Render(m_CurrentCamera, projection);
	}

    for (auto const& [name, instance] : m_InstancedQuad) {
        instance->Render(m_CurrentCamera, projection);
    }

    for (auto const& [name, instance] : m_InstancedModels) {
        instance->Render(m_CurrentCamera, projection);
    }
}

void Renderer::GatherImGui() {
    // frame counter
    if (!ImGui::Begin(U8_CAST("Візуалізатор"), &open))
    {
        ImGui::End();
        return;
    }

    auto get_texture_type_repr = [](unsigned int T) -> std::string {
        switch (T) {
        case GL_RED:
            return "GL_RED";
        case GL_RGB:
            return "GL_RGB";
        case GL_RGBA:
            return "GL_RGBA";
        default:
            return std::to_string(T);
        }
    };

    if (ImGui::CollapsingHeader(U8_CAST("Текстури"), &open)) {
        for (const auto& [name, tex] : m_Textures) {
            ImGui::Image((ImTextureID)(uint64_t)tex->id, ImVec2(48, 48), ImVec2(0, 0), ImVec2(1, 1));
            ImGui::SameLine();

            ImGui::BeginGroup();
            ImGui::Text("Texture: <%d> %s", tex->id, name.c_str());
            ImGui::Text("Type: %s", get_texture_type_repr(tex->gl_type));
            ImGui::Text("Usage: %s", tex->type.c_str());
            ImGui::Text("Path: %s", tex->path.c_str());
            ImGui::EndGroup();
            ImGui::Separator();
        }
    }

    if (ImGui::CollapsingHeader(U8_CAST("Шейдери"), &open)) {
        for (const auto& [name, shader] : m_Shaders) {
            if (ImGui::TreeNode(name.c_str())) {
                ImGui::Text("ID: %d", shader->ID);
                ImGui::TreePop();
            }
        }
    }

    if (ImGui::CollapsingHeader(U8_CAST("Камери"), &open)) {
        std::string camera_name;
        for (const auto& [name, camera] : m_Cameras) {
            if (camera == m_CurrentCamera) {
                camera_name = "[SELECTED]" + name;
            }
            else {
                camera_name = name;
            }
            if (ImGui::TreeNode(camera_name.c_str())) {
                camera->UI_Description();
                ImGui::TreePop();
            }
        }
    }

    if (ImGui::CollapsingHeader(U8_CAST("Базові сутності"), &open)) {
        for (auto object : m_Objects) {
            object->UI_Description();
            ImGui::Separator();
        }
    }

    if (ImGui::CollapsingHeader(U8_CAST("Груповані сутності"), &open)) {
        for (const auto& [name, instance_controller] : m_InstancedQuad) {
            instance_controller->UI_Description();
            ImGui::Separator();
        }
        for (const auto& [name, instance_controller] : m_InstancedModels) {
            instance_controller->UI_Description();
            ImGui::Separator();
        }
    }

    if (ImGui::CollapsingHeader(U8_CAST("Налаштування"), &open)) {
        ImGui::DragFloat(U8_CAST("Константа"), &m_Settings.light_constant, 0.1f, 1.0f, 100.f);
        ImGui::DragFloat(U8_CAST("Освіт. лінійний коеф."), &m_Settings.light_linear, 0.05f, 0.01f, 100.f);
        ImGui::DragFloat(U8_CAST("Освіт. квадратичний коеф."), &m_Settings.light_quadratic, 0.05f, 0.01f, 100.f);
        ImGui::DragFloat(U8_CAST("Освіт. інтенсивність"), &m_Settings.intensity, 0.2f, 0.2f, 20.f);
        ImGui::DragFloat(U8_CAST("Освіт. окружне"), &m_Settings.ambient, 0.05f, 0.01f, 1.f);
        ImGui::DragFloat(U8_CAST("Експозиція HDR"), &m_Settings.exposure, 0.05f, 0.0f, 2.f);
        ImGui::DragFloat(U8_CAST("Поріг блуму"), &m_Settings.bloom_threshold, 0.05f, 0.0f, 2.f);
        ImGui::DragInt(U8_CAST("Проходів блуму"), &m_Settings.bloom_radius, 2, 2, 30);
        ImGui::DragFloat(U8_CAST("Радіус фрустума"), &m_Settings.models_sphere_radius, 0.05f, 0.01f, 5.f);
        ImGui::DragInt(U8_CAST("Кількість джерел світла"), &m_Settings.current_light_limits, 1, 0, 256);
        ImGui::Checkbox(U8_CAST("Режим каркасу"), &m_Settings.wireframe);
    }

    ImGui::End();
}

std::shared_ptr<Shader> Renderer::GetShader(std::string shader_name) {
	auto val = m_Shaders.find(shader_name);
	if (val != m_Shaders.end()) {
		return val->second;
	}
    Logger::instance().AddLog("[Renderer] Shader lookup failed, desired name: %s\n", shader_name.c_str());
	return nullptr;
}

std::shared_ptr<Texture> Renderer::GetTexture(std::string texture_name) {
	auto val = m_Textures.find(texture_name);
	if (val != m_Textures.end()) {
		return val->second;
	}
    Logger::instance().AddLog("[Renderer] Texture lookup failed, desired name: %s\n", texture_name.c_str());
	return nullptr;
}

std::shared_ptr<InstancedQuad> Renderer::GetInstancedQuad(std::string instance_name) {
    auto val = m_InstancedQuad.find(instance_name);
    if (val != m_InstancedQuad.end()) {
        return val->second;
    }
    Logger::instance().AddLog("[Renderer] Instanced Quad lookup failed, desired name: %s\n", instance_name.c_str());
    return nullptr;
}


std::shared_ptr<Texture> Renderer::NewTexture(const char* file_path, std::string name, std::string type, const bool compress) {
    auto [tex_id, tex_type] = m_loadTexture(file_path, true, compress);
    std::cout << "new texture, " << tex_id << " | " << tex_type << std::endl;
    std::shared_ptr<Texture> texture = nullptr;

    if (tex_id != 0) [[likely]] {
        texture = std::shared_ptr<Texture>(new Texture(tex_id, tex_type, type, file_path));

        auto ret = m_Textures.insert({
            name,
            texture
            });

        if (!ret.second) {
            Logger::instance().AddLog("[Renderer] Texture map insertion failed, name: %s\n", name.c_str());
        }
        else [[likely]] {
            Logger::instance().AddLog("[Renderer] Created texture <%d> with name: %s\n", tex_id, name.c_str());
        }
    }
    else {
        texture = GetTexture("missing");
        Logger::instance().AddLog("[Renderer] Replaced with default missing texture");
    }

    return texture;
}


std::tuple<unsigned int, unsigned int> m_loadTexture(const char* path, const bool flip, const bool compress)
{
    std::cout << "m_LoadTexture compress state: " << compress << std::endl;
    unsigned int textureID;
    glGenTextures(1, &textureID);

    stbi_set_flip_vertically_on_load(flip);

    GLenum format;
    GLint internal_format;
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        if (nrComponents == 1) {
            format = GL_RED;
            internal_format = GL_RED;
        }
        else if (nrComponents == 3) {
            format = GL_RGB;
            internal_format = compress ? GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_RGB;
        }
        else if (nrComponents == 4) {
            format = GL_RGBA;
            internal_format = compress ? GL_COMPRESSED_RGBA_S3TC_DXT3_EXT : GL_RGBA;
        }
        std::cout << "m_LoadTexture formats: " << format << " | " << internal_format << std::endl;
        glBindTexture(GL_TEXTURE_2D, textureID);

        // glCompressedTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, sizeof(data) * width * height, data);
        GLClearErrors();
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        GLGetErrors();
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "[Renderer] Texture loading failed, path: " << path << std::endl;
        Logger::instance().AddLog("[Renderer] Texture loading failed, path: %s\n", path);
        stbi_image_free(data);
        return { 0, 0 };
    }
    stbi_set_flip_vertically_on_load(true);
    return { textureID, format };
}


std::shared_ptr<Camera::BaseCamera> Renderer::NewCamera(glm::vec3 coords, std::string camera_name, Camera::Camera_Type cam_type) {
    switch (cam_type) {
        case Camera::Camera_Type::FLYCAM:
        {
            auto fly_cam = std::shared_ptr<Camera::FlyCamera>(new Camera::FlyCamera(coords));
            m_Cameras.insert({ camera_name, fly_cam });
            // TODO: technically, we do want to check for insert result to ensure we don't create a new camera with the same name
            // doesn't really matter rn since we don't have a strong need in many cameras at once
            Logger::instance().AddLog("[Renderer] New Fly camera with name %s\n", camera_name);
            return fly_cam;
        }
        case Camera::Camera_Type::ARCBALL:
        {
            auto arcball_cam = std::shared_ptr<Camera::ArcballCamera>(new Camera::ArcballCamera(coords));
            m_Cameras.insert({ camera_name, arcball_cam });
            Logger::instance().AddLog("[Renderer] New Arcball camera with name %s\n", camera_name);
            return arcball_cam;
        }
    }
    return nullptr;
}

bool Renderer::SetActiveCamera(std::string camera_name) {
    auto val = m_Cameras.find(camera_name);
    if (val != m_Cameras.end()) {
        m_CurrentCamera = val->second;
        m_CurrentCameraType = m_CurrentCamera->m_CamType;
        return true;
    }
    else {
        Logger::instance().AddLog("[Renderer] Camera lookup failed, desired name: %s\n", camera_name);
        return false;
    }
}

void Renderer::SimpleQuad() {
    if (m_SimpleQuadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &m_SimpleQuadVAO);
        glGenBuffers(1, &m_SimpleQuadVBO);
        glBindVertexArray(m_SimpleQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_SimpleQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(m_SimpleQuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


std::shared_ptr<Texture> Renderer::NewCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    stbi_set_flip_vertically_on_load(false);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    stbi_set_flip_vertically_on_load(true);

    auto cubemap_tex = std::shared_ptr<Texture>(new Texture(textureID, GL_TEXTURE_CUBE_MAP, "cubemap", ""));

    m_Textures.insert({ 
        "cubemap", 
        cubemap_tex
    });

    return cubemap_tex;
}


void Renderer::Reset() {
    std::cout << "[Renderer] Reset" << std::endl;
    m_CurrentCamera = nullptr;
    m_Cameras.clear();
    m_InstancedQuad.clear();
    m_InstancedModels.clear();
    m_Objects.clear();

    m_Shaders.clear();
    m_Textures.clear();

    NewTexture(
        "Resource/textures/missing.jpg", "missing", "texture_diffuse"
    );
}


void Renderer::PerfCounter() {
    static const float DISTANCE = 10.0f;
    static bool open = true;
    static int corner = 1;

    ImGuiIO& io = ImGui::GetIO();
    if (corner != -1)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_area_pos = ImGui::GetWindowPos();   // Instead of using viewport->Pos we use GetWorkPos() to avoid menu bars, if any!
        ImVec2 work_area_size = ImGui::GetWindowSize();

        ImVec2 window_pos = ImVec2(
            (corner & 1) ? (work_area_pos.x + work_area_size.x) : (work_area_pos.x + DISTANCE),
            (corner & 2) ? (work_area_pos.y + work_area_size.y - DISTANCE) : (work_area_pos.y + DISTANCE * 2.f)
        );
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (corner != -1)
        window_flags |= ImGuiWindowFlags_NoMove;
    if (ImGui::Begin(U8_CAST("Швидкість виконання"), &open, window_flags))
    {
        ImGui::Text(U8_CAST("Час на кадр: %.2f мс"), m_FrameTime * 1000.0);
        ImGui::Text(U8_CAST("FPS: %.1f"), 1.0 / m_FrameTime);
        if(m_VisibleLights > 0)
            ImGui::Text(U8_CAST("Видимі джерела світла: %d"), m_VisibleLights);
        if(m_VisibleModels > 0)
            ImGui::Text(U8_CAST("Видимі об'єкти: %d"), m_VisibleModels);
    }
    ImGui::End();
}


void Renderer::Dockspace() {
    static bool p_open = true;
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetWorkPos());
        ImGui::SetNextWindowSize(viewport->GetWorkSize());
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", &p_open, window_flags);
    ImGui::PopStyleVar();

    if (opt_fullscreen)
        ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO();
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    ImGui::End();
}