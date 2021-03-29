#include "Renderer.h"
#include "../stb_image.h"
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

unsigned int loadTexture(char const* path, const bool flip);
static bool open = true;

namespace Rendering {
    unsigned int SCREEN_WIDTH = 1920;
    unsigned int SCREEN_HEIGHT = 1060;
}

Renderer::Renderer() {
    std::cout << "Renderer created" << std::endl;
}


Model& Renderer::NewModel() {

}

std::shared_ptr<Quad> Renderer::NewQuad(std::string texture_name, std::string shader_name) {
    std::cout << "New quad at " << this << std::endl;
	Shader* shader = this->GetShader(shader_name);
	Texture* texture = this->GetTexture(texture_name);

    auto quad = std::shared_ptr<Quad>(new Quad(texture, shader));
	
    objects.push_back(quad);

	return quad;
}

std::shared_ptr<InstancedQuad> Renderer::NewInstancedQuad(std::string texture_name, std::string shader_name, std::string instance_name) {
    Shader* shader = this->GetShader(shader_name);
    Texture* texture = this->GetTexture(texture_name);

    auto instance = std::shared_ptr<InstancedQuad>(new InstancedQuad(texture, shader));
    auto ret = instanced_quads.insert({instance_name, instance});

    return instance;
}


std::shared_ptr<InstancedModel> Renderer::NewInstancedModel(std::string model_path, std::string shader_name, std::string instance_name) {
    Shader* shader = this->GetShader(shader_name);

    auto instance = std::shared_ptr<InstancedModel>(new InstancedModel(model_path.c_str(), shader));
    auto ret = instanced_models.insert({ instance_name, instance });

    return instance;
}

Shader* Renderer::NewShader(const char* vertex_path, const char* fragment_path, std::string name) {
    std::cout << "New shader at " << this << std::endl;
	auto ret = shaders.insert({ name, Shader(vertex_path, fragment_path, name) });
	return &ret.first->second;
}

Shader* Renderer::NewShader(const char* vertex_path, const char* fragment_path, const char* geometry_pass, std::string name) {
    std::cout << "Creating new shader of type " << name << std::endl;
    auto ret = shaders.insert({ name, Shader(vertex_path, fragment_path, geometry_pass, name) });
    return &ret.first->second;
}


Shader* Renderer::NewShader(const char* vertex_path, const char* fragment_path, const char* tess_control_path, const char* tess_eval_path, std::string name) {
    std::cout << "Creating new shader of type " << name << std::endl;
    auto ret = shaders.insert({ name, Shader(vertex_path, fragment_path, tess_control_path, tess_eval_path, name) });
    return &ret.first->second;
}


void Renderer::Render() {
    glm::mat4 projection = glm::perspective(
        glm::radians(current_camera->m_Zoom), 
        (float)Rendering::SCREEN_WIDTH / (float)Rendering::SCREEN_HEIGHT, 0.1f, 100.0f);

	for (auto& obj : objects) {
		obj->Render(current_camera, projection);
	}
    for (auto const& [name, instance] : instanced_quads) {
        instance->Render(current_camera, projection);
    }

    for (auto const& [name, instance] : instanced_models) {
        instance->Render(current_camera, projection);
    }
}

void Renderer::GatherImGui() {
    // frame counter
    {
        const float DISTANCE = 10.0f;
        static int corner = 1;
        ImGuiIO& io = ImGui::GetIO();
        if (corner != -1)
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 work_area_pos = viewport->GetWorkPos();   // Instead of using viewport->Pos we use GetWorkPos() to avoid menu bars, if any!
            ImVec2 work_area_size = viewport->GetWorkSize();
            ImVec2 window_pos = ImVec2((corner & 1) ? (work_area_pos.x + work_area_size.x - DISTANCE) : (work_area_pos.x + DISTANCE), (corner & 2) ? (work_area_pos.y + work_area_size.y - DISTANCE) : (work_area_pos.y + DISTANCE));
            ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            ImGui::SetNextWindowViewport(viewport->ID);
        }
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (corner != -1)
            window_flags |= ImGuiWindowFlags_NoMove;
        if (ImGui::Begin("Renderer: Frame Stats", &open, window_flags))
        {
            ImGui::Text("Frame Time: %.2f ms", this->frame_time * 1000.0);
            ImGui::Text("FPS: %.1f", 1.0 / this->frame_time);
            ImGui::Text("Visible Light: %d", this->visible_lights);
            ImGui::Text("Visible Models: %d", this->visible_models);
        }
        ImGui::End();
    }

    if (!ImGui::Begin("Renderer", &open))
    {
        ImGui::End();
        return;
    }

    if (ImGui::CollapsingHeader("Textures", &open)) {
        for (const auto& [name, tex] : textures) {
            ImGui::Image((ImTextureID)tex.id, ImVec2(48, 48), ImVec2(0, 0), ImVec2(1, 1));
            ImGui::SameLine();

            ImGui::BeginGroup();
            ImGui::Text("Texture: <%d> %s", tex.id, name.c_str());
            ImGui::Text("Texture type: %s", tex.type.c_str());
            ImGui::Text("Texture path: %s", tex.path.c_str());
            ImGui::EndGroup();
            ImGui::Separator();
        }
    }

    if (ImGui::CollapsingHeader("Shaders", &open)) {
        for (const auto& [name, shader] : shaders) {
            if (ImGui::TreeNode(name.c_str())) {
                ImGui::Text("ID: %d", shader.ID);
                ImGui::TreePop();
            }
        }
    }

    if (ImGui::CollapsingHeader("Cameras", &open)) {
        std::string camera_name;
        for (const auto& [name, camera] : cameras) {
            if (camera == current_camera) {
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
        for (auto object : objects) {
            object->UI_Description();
            ImGui::Separator();
        }
    }

    if (ImGui::CollapsingHeader("Instanced Entities", &open)) {
        for (const auto& [name, instance_controller] : instanced_quads) {
            instance_controller->UI_Description();
            ImGui::Separator();
        }
        for (const auto& [name, instance_controller] : instanced_models) {
            instance_controller->UI_Description();
            ImGui::Separator();
        }
    }

    if (ImGui::CollapsingHeader("Renderer Settings", &open)) {
        /*
        int light_count = 32;
		float light_spread = 6;

		float light_constant = 1.0;
		float light_linear = 0.7;
		float light_quadratic = 1.4;

		bool wireframe = false;
        */
        ImGui::DragInt("Light Count", &settings.light_count, 1, 0, 256);
        ImGui::DragFloat("Light Constant", &settings.light_constant, 0.1, 1.0, 100.f);
        ImGui::DragFloat("Light Linear", &settings.light_linear, 0.05, 0.01, 100.f);
        ImGui::DragFloat("Light Quadratic", &settings.light_quadratic, 0.1, 1.0, 100.f);
        ImGui::DragFloat("Model Frustum Radius", &settings.models_sphere_radius, 0.05, 0.01, 5.f);
        ImGui::Checkbox("Wireframe", &settings.wireframe);
    }

    ImGui::End();
}

Shader* Renderer::GetShader(std::string shader_name) {
	auto val = shaders.find(shader_name);
	if (val != shaders.end()) {
		return &val->second;
	}
	return nullptr;
}

Texture* Renderer::GetTexture(std::string texture_name) {
	auto val = textures.find(texture_name);
	if (val != textures.end()) {
		return &val->second;
	}
	return nullptr;
}

std::shared_ptr<InstancedQuad> Renderer::GetInstancedQuad(std::string instance_name) {
    auto val = instanced_quads.find(instance_name);
    if (val != instanced_quads.end()) {
        return val->second;
    }
    return nullptr;
}


Texture* Renderer::NewTexture(const char* file_path, std::string name, std::string type) {
    unsigned int tex_id = loadTexture(file_path, true);

    auto ret = textures.insert(
        { 
        name, 
            {
            tex_id, type, file_path
            }
        }
    );
    return &ret.first->second;
}


unsigned int loadTexture(char const* path, const bool flip = true)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    stbi_set_flip_vertically_on_load(flip);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    stbi_set_flip_vertically_on_load(true);
    return textureID;
}


std::shared_ptr<Camera::BaseCamera> Renderer::NewCamera(glm::vec3 coords, std::string camera_name, Camera::Camera_Type cam_type) {
    switch (cam_type) {
    case Camera::Camera_Type::FLYCAM:
    {
        auto fly_cam = std::shared_ptr<Camera::FlyCamera>(new Camera::FlyCamera(coords));
        cameras.insert({ camera_name, fly_cam });
        // TODO: technically, we do want to check for insert result to ensure we don't create a new camera with the same name
        // doesn't really matter rn since we don't have a strong need in many cameras at once
        return fly_cam;
    }
    case Camera::Camera_Type::ARCBALL:
    {
        auto arcball_cam = std::shared_ptr<Camera::ArcballCamera>(new Camera::ArcballCamera(coords));
        cameras.insert({ camera_name, arcball_cam });
        return arcball_cam;
    }
    }
}

void Renderer::SetActiveCamera(std::string camera_name) {
    std::cout << "cameras vector length: " << cameras.size() << std::endl;
    auto val = cameras.find(camera_name);
    if (val != cameras.end()) {
        current_camera = val->second;
        current_camera_type = current_camera->m_CamType;
        std::cout << "using camera " << current_camera << " with name " << camera_name << std::endl;
    }
    else {
        std::cout << "camera with name " << camera_name << " was not found!" << std::endl;
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


unsigned int loadCubemap(std::vector<std::string> faces)
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

    return textureID;
}