#include "Lab2Scene.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <imgui/imgui.h>

#include <random>

#include "../Renderer/Renderer.h"
#include "../ImGui/ImGuiLogger.h"

void Lab2Scene::Setup() {
    Logger* logger = &Logger::instance();
    logger->AddLog("[Lab1] setup started...\n");
    Renderer* renderer = &Renderer::instance();

    auto arcball_camera = renderer->NewCamera({ 0.0f, 0.0f, 3.0f }, "arcball_camera", Camera::Camera_Type::ARCBALL);
    renderer->SetActiveCamera("arcball_camera");

    tess_shader = renderer->NewShader(
        "Shaders/Tessellation/tessellation_vertex.glsl", 
        "Shaders/Tessellation/tessellation_fragment.glsl", 
        "Shaders/Tessellation/tessellation_control.glsl", 
        "Shaders/Tessellation/tessellation_evaluation.glsl", 
        "l_box"
    );

    // define shape here

    this->RebuildMesh();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, this->mesh.size() * sizeof(BasicVertex), &this->mesh[0], GL_STREAM_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BasicVertex), (GLvoid*)offsetof(BasicVertex, m_Position));
    glEnableVertexAttribArray(0);

    glDepthFunc(GL_LEQUAL);
    glPatchParameteri(GL_PATCH_VERTICES, 16);
}

void Lab2Scene::Render() {
    Renderer* renderer = &Renderer::instance();

    auto camera = renderer->m_CurrentCamera;

    glm::mat4 projection = glm::perspective(
        glm::radians(renderer->m_CurrentCamera->m_Zoom),
        (float)Rendering::SCREEN_WIDTH / (float)Rendering::SCREEN_HEIGHT,
        0.1f, 100.0f
    );

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model(1.f);
    model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));

    tess_shader->use();
    tess_shader->setMat4("view", camera->GetViewMatrix());
    tess_shader->setMat4("projection", projection);
    tess_shader->setMat4("model", glm::mat4(1.f));
    tess_shader->setInt("inner", tess_inner);
    tess_shader->setInt("outer", tess_outer);
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, this->mesh.size() * sizeof(BasicVertex), &this->mesh[0], GL_STREAM_DRAW);

    glDrawArrays(GL_PATCHES, 0, this->mesh.size());

    if (ImGui::Begin("Controls")) {
        ImGui::InputInt("tess outer", &tess_outer);
        ImGui::InputInt("tess inner", &tess_inner);

        ImGui::End();
    }
    else {
        ImGui::End();
    }

    ImGui::ShowDemoWindow();

    glBindVertexArray(0);
}


void Lab2Scene::RebuildMesh() {
    if (!params_changed) return;
    mesh.clear();
    params_changed = false;

    mesh.push_back(BasicVertex{glm::vec3(-2.0f, -2.0f, 0.0f)});
    mesh.push_back(BasicVertex{glm::vec3(-1.0f, -2.0f, 1.0f)});
    mesh.push_back(BasicVertex{glm::vec3(1.0f, -1.0f, 2.0f)});
    mesh.push_back(BasicVertex{glm::vec3(2.0f, -2.0f, 0.0f)});
    mesh.push_back(BasicVertex{glm::vec3(-2.0f, -1.0f, 1.0f)});
    mesh.push_back(BasicVertex{glm::vec3(-1.0f, -1.0f, 1.0f)});
    mesh.push_back(BasicVertex{glm::vec3(2.0f, 0.0f, 1.0)});
    mesh.push_back(BasicVertex{glm::vec3(2.0f, -1.0f, 2.0f)});
    mesh.push_back(BasicVertex{glm::vec3(-3.0f, 0.0f, 1.0f)});
    mesh.push_back(BasicVertex{glm::vec3(-1.0f, -1.5f, 1.0f)});
    mesh.push_back(BasicVertex{glm::vec3(0.0f, 0.0f, 0.0f)});
    mesh.push_back(BasicVertex{glm::vec3(1.0f, 1.0f, 1.0f)});
    mesh.push_back(BasicVertex{glm::vec3(-2.0f, 2.0f, 0.0f)});
    mesh.push_back(BasicVertex{glm::vec3(-1.5f, 3.0f, 2.0f)});
    mesh.push_back(BasicVertex{glm::vec3(1.0f, 3.0f, 2.0f)});
    mesh.push_back(BasicVertex{glm::vec3(2.0f, 2.0f, 0.0f)});
}