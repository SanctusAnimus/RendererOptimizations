#include "InstancedQuad.h"

#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

#include "../Renderer.h"
#include "../../ImGui/ImGuiLogger.h"

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


InstancedQuad::InstancedQuad(Texture* texture, std::shared_ptr<Shader> shader): m_Texture(texture), m_Shader(shader) {
    glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
    glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
    glm::vec3 pos3(1.0f, -1.0f, 0.0f);
    glm::vec3 pos4(1.0f, 1.0f, 0.0f);
    // texture coordinates
    glm::vec2 uv1(0.0f, 1.0f);
    glm::vec2 uv2(0.0f, 0.0f);
    glm::vec2 uv3(1.0f, 0.0f);
    glm::vec2 uv4(1.0f, 1.0f);
    // normal vector
    glm::vec3 nm(0.0f, 0.0f, 1.0f);

    // calculate tangent/bitangent vectors of both triangles
    glm::vec3 tangent1, bitangent1;
    glm::vec3 tangent2, bitangent2;
    // triangle 1
    // ----------
    glm::vec3 edge1 = pos2 - pos1;
    glm::vec3 edge2 = pos3 - pos1;
    glm::vec2 deltaUV1 = uv2 - uv1;
    glm::vec2 deltaUV2 = uv3 - uv1;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

    bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

    // triangle 2
    // ----------
    edge1 = pos3 - pos1;
    edge2 = pos4 - pos1;
    deltaUV1 = uv3 - uv1;
    deltaUV2 = uv4 - uv1;

    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


    bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


    float quadVertices[] = {
        pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
    };

    unsigned int indices[] = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    // configure plane VAO
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    glGenBuffers(1, &m_ABO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, m_ABO);
    glBufferData(GL_ARRAY_BUFFER, models.size() * 4 * sizeof(glm::vec4), &models[0], GL_DYNAMIC_DRAW);

    unsigned int attrib_size = 14 * sizeof(float);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attrib_size, (void*)0);
    glVertexAttribDivisor(0, 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, attrib_size, (void*)(3 * sizeof(float)));
    glVertexAttribDivisor(1, 0);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, attrib_size, (void*)(6 * sizeof(float)));
    glVertexAttribDivisor(2, 0);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, attrib_size, (void*)(8 * sizeof(float)));
    glVertexAttribDivisor(3, 0);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, attrib_size, (void*)(11 * sizeof(float)));
    glVertexAttribDivisor(4, 0);

    std::size_t vec4Size = sizeof(glm::vec4);
    glBindBuffer(GL_ARRAY_BUFFER, m_ABO);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glVertexAttribDivisor(5, 1);

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glVertexAttribDivisor(6, 1);

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glVertexAttribDivisor(7, 1);

    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
    glVertexAttribDivisor(8, 1);

    glBindVertexArray(0);

    m_BuffersInit = true;
}

void InstancedQuad::Add(glm::mat4 position) {
    models.push_back(position);
    GLClearErrors();
    glBindBuffer(GL_ARRAY_BUFFER, m_ABO);
    glBufferData(GL_ARRAY_BUFFER, models.size() * 64, &models[0], GL_DYNAMIC_DRAW);
    GLGetErrors();

    Logger::instance().AddLog("[InstancedQuad] Added new instance of quad\n");
}

void InstancedQuad::Render(std::shared_ptr<Camera::BaseCamera> camera, glm::mat4 projection) {
    this->m_Shader->use();
    this->m_Shader->setVec3("viewPos", camera->m_Position);
    this->m_Shader->setMat4("view", camera->GetViewMatrix());
    this->m_Shader->setMat4("projection", projection);

    int tex_num = 0;

    glActiveTexture(GL_TEXTURE0);
    this->m_Shader->setInt("texture_diffuse1", 0);
    glBindTexture(GL_TEXTURE_2D, this->m_Texture->id);

    if (m_NormalMap != nullptr) {
        glActiveTexture(GL_TEXTURE0 + ++tex_num);
        this->m_Shader->setInt("texture_normal1", tex_num);
        glBindTexture(GL_TEXTURE_2D, this->m_NormalMap->id);
    }

    if (m_SpecularMap != nullptr) {
        glActiveTexture(GL_TEXTURE0 + ++tex_num);
        this->m_Shader->setInt("texture_specular1", tex_num);
        glBindTexture(GL_TEXTURE_2D, this->m_SpecularMap->id);
    }

    if (m_HeightMap != nullptr) {
        glActiveTexture(GL_TEXTURE0 + ++tex_num);
        this->m_Shader->setInt("texture_height1", tex_num);
        glBindTexture(GL_TEXTURE_2D, this->m_HeightMap->id);
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_ABO);
    // glBufferData(GL_ARRAY_BUFFER, models.size() * 4 * sizeof(glm::vec4), &models[0], GL_DYNAMIC_DRAW);

    glBindVertexArray(m_VAO);

    // glm::mat4 model(1.f);
    // this->m_Shader->setMat4("normal_model", model);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, models.size());
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
}


void InstancedQuad::SetNormalMap(std::string tex_name) {
    m_NormalMap = Renderer::instance().GetTexture(tex_name);
    std::cout << "set normal map to " << m_NormalMap << std::endl;
}

void InstancedQuad::SetSpecularMap(std::string tex_name) {
    m_SpecularMap = Renderer::instance().GetTexture(tex_name);
    std::cout << "set specular map to " << m_SpecularMap << std::endl;
}

void InstancedQuad::SetHeightMap(std::string tex_name) {
    m_HeightMap = Renderer::instance().GetTexture(tex_name);
    std::cout << "set height map to " << m_HeightMap << std::endl;
}


void InstancedQuad::UI_Description() {
    ImGui::TextUnformatted("Type: Quad");
    ImGui::Text("Controlled entities: %d", models.size());
    ImGui::Text("Diffuse: %s", m_Texture->path.c_str());
    if (m_NormalMap != nullptr) {
        ImGui::Text("Normal: %s", m_NormalMap->path.c_str());
    }
    if (m_SpecularMap != nullptr) {
        ImGui::Text("Specular: %s", m_SpecularMap->path.c_str());
    }    
    if (m_HeightMap != nullptr) {
        ImGui::Text("Height: %s", m_HeightMap->path.c_str());
    }

    if (ImGui::Button("Add"))
        ImGui::OpenPopup("new_instanced_entity");

    if (ImGui::BeginPopup("new_instanced_entity")) {
        static glm::vec3 translate(1.f);
        static glm::vec3 rotate_vec(1.f, 0.f, 0.f);
        static glm::vec3 scale(1.f);
        static float degrees = 0;

        ImGui::TextUnformatted("Create new instance of Quad");
        // models and such;
        ImGui::DragFloat3("Translate", &translate.x, 0.5);
        ImGui::DragFloat3("Scale", &scale.x, 0.5);
        ImGui::InputFloat3("Rotate Vector", &rotate_vec.x);
        ImGui::InputFloat("Degrees", &degrees);

        if (ImGui::Button("Create")) {
            glm::mat4 qmodel = glm::mat4(1.0f);
            qmodel = glm::translate(qmodel, translate);
            qmodel = glm::rotate(qmodel, glm::radians(degrees), rotate_vec);
            qmodel = glm::scale(qmodel, scale);
            this->Add(qmodel);
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}