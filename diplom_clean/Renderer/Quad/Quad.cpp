#include "Quad.h"
#include "../Renderer.h"
#include <imgui/imgui.h>


Quad::Quad(std::shared_ptr<Texture> texture, std::shared_ptr<Shader> shader) : Renderable(texture, shader) {
    Rebuild();
}


void Quad::Render(std::shared_ptr<Camera::BaseCamera> camera, glm::mat4 projection)
{
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

    glBindVertexArray(m_VAO);

    glm::mat4 model(1.f);
    model = glm::translate(model, m_Transform);
    model = glm::rotate(model, m_RotationRadians, m_Rotation);
    model = glm::scale(model, m_Scale);
    this->m_Shader->setMat4("model", model);
    model = glm::inverse(model);
    model = glm::transpose(model);
    this->m_Shader->setMat4("normal_model", model);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    for (int i = 0; i <= tex_num; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}


void Quad::UI_Description() {
    ImGui::PushID(m_VAO);
    ImGui::TextUnformatted(U8_CAST("Квад"));
    ImGui::DragFloat3(U8_CAST("Координати"), &m_Transform.x, 0.5f, -100.f, 100.f);
    ImGui::InputFloat3(U8_CAST("Масштаб"), &m_Scale.x, 1);
    ImGui::InputFloat3(U8_CAST("Вектор повороту"), &m_Rotation.x, 1);
    ImGui::InputFloat(U8_CAST("Кут (радіани)"), &m_RotationRadians, 0.0, 1.f, 1);
    ImGui::PopID();
}


void Quad::SetUV(glm::vec2 uv1, glm::vec2 uv2, glm::vec2 uv3, glm::vec2 uv4) {
    m_UV1 = uv1;
    m_UV2 = uv2;
    m_UV3 = uv3;
    m_UV4 = uv4;
               
    Rebuild();
}


void Quad::Rebuild() {
    // positions
    glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
    glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
    glm::vec3 pos3(1.0f, -1.0f, 0.0f);
    glm::vec3 pos4(1.0f, 1.0f, 0.0f);
    // normal vector
    glm::vec3 nm(0.0f, 0.0f, 1.0f);

    // calculate tangent/bitangent vectors of both triangles
    glm::vec3 tangent1, bitangent1;
    glm::vec3 tangent2, bitangent2;
    // triangle 1
    // ----------
    glm::vec3 edge1 = pos2 - pos1;
    glm::vec3 edge2 = pos3 - pos1;
    glm::vec2 deltaUV1 = m_UV2 - m_UV1;
    glm::vec2 deltaUV2 = m_UV3 - m_UV1;

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
    deltaUV1 = m_UV3 - m_UV1;
    deltaUV2 = m_UV4 - m_UV1;

    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


    bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


    float quadVertices[] = {
        pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, m_UV1.x, m_UV1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, m_UV2.x, m_UV2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, m_UV3.x, m_UV3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, m_UV4.x, m_UV4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
    };

    unsigned int indices[] = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    // configure plane VAO
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
}