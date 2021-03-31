#include "Lab1Scene.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <imgui/imgui.h>

#include <random>

#include "../Renderer/Renderer.h"
#include "../ImGui/ImGuiLogger.h"

float rotate(float x, float y, float angle, float height) {

}

void Lab1Scene::Setup() {
    Logger* logger = &Logger::instance();
    logger->AddLog("[Lab1] setup started...\n");
    Renderer* renderer = &Renderer::instance();

    auto arcball_camera = renderer->NewCamera({ 0.0f, 0.0f, 3.0f }, "arcball_camera", Camera::Camera_Type::ARCBALL);
    renderer->SetActiveCamera("arcball_camera");

    light_box_shader = renderer->NewShader(
        "Shaders/Deferred/light_box_vertex.glsl", "Shaders/Deferred/light_box_fragment.glsl", "l_box");

    // define shape here

    this->BuildMesh();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, this->mesh.size() * sizeof(BasicVertex), &this->mesh[0], GL_STREAM_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BasicVertex), (GLvoid*)offsetof(BasicVertex, m_Position));
    glEnableVertexAttribArray(0);
}

void Lab1Scene::Render() {
    Renderer* renderer = &Renderer::instance();

    auto camera = renderer->m_CurrentCamera;

    glm::mat4 projection = glm::perspective(
        glm::radians(renderer->m_CurrentCamera->m_Zoom),
        (float)Rendering::SCREEN_WIDTH / (float)Rendering::SCREEN_HEIGHT,
        0.1f, 100.0f
    );

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    light_box_shader->use();
    light_box_shader->setMat4("view", camera->GetViewMatrix());
    light_box_shader->setMat4("projection", projection);
    light_box_shader->setMat4("model", glm::mat4(1.f));
    light_box_shader->setVec3("lightColor", glm::vec3(0.5, 0.1, 0.7));
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, this->mesh.size() * sizeof(BasicVertex), &this->mesh[0], GL_STREAM_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, this->mesh.size());

    glBindVertexArray(0);
}


void Lab1Scene::BuildMesh() {
    if (!params_changed) return;
    this->mesh.clear();

    float step = 0.1;
    float x_range = 2;
    float y_range = 2;
    float radius = 0.5f;

    auto GetZ = [radius](float x, float y) {return sqrt(radius * radius - x * x - y * y); };



    for (float x = 0; x <= 2; x += 2 * step) {
        for (float y = 0; y <= 1; y += step) {
            float _x = x * 3.14;
            float _y = y * 3.14;

            float next_x = (x + step) * 3.14;
            float next_y = (y + step) * 3.14;


            this->mesh.push_back(
                BasicVertex{ glm::vec3(x, y, GetZ(x, y)) }
            );
            this->mesh.push_back(
                BasicVertex{ glm::vec3(next_x, y, GetZ(next_x, y)) }
            );
            this->mesh.push_back(
                BasicVertex{ glm::vec3(next_x, next_y, GetZ(next_x, next_y)) }
            );

            //second triangle
            this->mesh.push_back(
                BasicVertex{ glm::vec3(x, y, GetZ(x, y)) }
            );
            this->mesh.push_back(
                BasicVertex{ glm::vec3(next_x, next_y, GetZ(next_x, next_y)) }
            );
            this->mesh.push_back(
                BasicVertex{ glm::vec3(x, next_y, GetZ(x, next_y)) }
            );
        }
    }

    params_changed = false;
}