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

    auto instance = std::shared_ptr<InstancedModel>(new InstancedModel(model_path.c_str(), shader));
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
    if (!ImGui::Begin("Renderer", &open))
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

    if (ImGui::CollapsingHeader("Textures", &open)) {
        for (const auto& [name, tex] : m_Textures) {
            ImGui::Image((ImTextureID)tex->id, ImVec2(48, 48), ImVec2(0, 0), ImVec2(1, 1));
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

    if (ImGui::CollapsingHeader("Shaders", &open)) {
        for (const auto& [name, shader] : m_Shaders) {
            if (ImGui::TreeNode(name.c_str())) {
                ImGui::Text("ID: %d", shader->ID);
                ImGui::TreePop();
            }
        }
    }

    if (ImGui::CollapsingHeader("Cameras", &open)) {
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

    if (ImGui::CollapsingHeader("Basic Entities", &open)) {
        for (auto object : m_Objects) {
            object->UI_Description();
            ImGui::Separator();
        }
    }

    if (ImGui::CollapsingHeader("Instanced Entities", &open)) {
        for (const auto& [name, instance_controller] : m_InstancedQuad) {
            instance_controller->UI_Description();
            ImGui::Separator();
        }
    }

    if (ImGui::CollapsingHeader("Renderer Settings", &open)) {
        ImGui::DragFloat("Light Constant", &m_Settings.light_constant, 0.1, 1.0, 100.f);
        ImGui::DragFloat("Light Linear", &m_Settings.light_linear, 0.05, 0.01, 100.f);
        ImGui::DragFloat("Light Quadratic", &m_Settings.light_quadratic, 0.05, 0.01, 100.f);
        ImGui::DragFloat("Light Intensity", &m_Settings.intensity, 0.2, 0.2, 20.f);
        ImGui::DragFloat("Light Ambient", &m_Settings.ambient, 0.05, 0.01, 1.f);
        ImGui::DragFloat("Light Spread", &m_Settings.light_spread, 1.f, 0.5f, 100.f);
        ImGui::DragFloat("HDR Exposure", &m_Settings.exposure, 0.05, 0.0, 2.f);
        ImGui::DragFloat("Bloom Threshold", &m_Settings.bloom_threshold, 0.05, 0.0, 2.f);
        ImGui::DragInt("Bloom Radius", &m_Settings.bloom_radius, 2.0, 2.0, 30.f);
        ImGui::DragFloat("Model Frustum Radius", &m_Settings.models_sphere_radius, 0.05, 0.01, 5.f);
        ImGui::Checkbox("Wireframe", &m_Settings.wireframe);
        ImGui::Checkbox("Skybox", &m_Settings.skybox);
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

    auto texture = std::shared_ptr<Texture>(new Texture(tex_id, tex_type, type, file_path));

    auto ret = m_Textures.insert({ 
        name, 
        texture
    });

    if (!ret.second) {
        Logger::instance().AddLog("[Renderer] Texture map insertion failed, name: %s\n", name.c_str());
    }
    else {
        Logger::instance().AddLog("[Renderer] Created texture <%d> with name: %s\n", tex_id, name.c_str());
    }
    return texture;
}


std::tuple<unsigned int, unsigned int> m_loadTexture(const char* path, const bool flip, const bool compress)
{
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
        Logger::instance().AddLog("[Renderer] Texture loading failed, path: %s\n", path);
        stbi_image_free(data);
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
}