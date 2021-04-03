#include "FirstLabScene.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <imgui/imgui.h>

#include <random>

#include <glm/gtx/rotate_vector.hpp>

#include "../Renderer/Renderer.h"
#include "../ImGui/ImGuiLogger.h"

float rotate(float x, float y, float angle, float height) {
    return 1.f;
}

Lab1Scene::~Lab1Scene() {
    unsigned int vaos[3] = { m_VAO, m_GridVAO, m_AxesVAO };
    unsigned int vbos[3] = { m_VBO, m_GridVBO, m_AxesVBO };
    glDeleteVertexArrays(3, vaos);
    glDeleteBuffers(3, vbos);
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

    points_shader = renderer->NewShader(
        "Shaders/Deferred/light_box_vertex.glsl",
        "Shaders/Deferred/light_box_fragment.glsl",
        "points"
    );

    axes_shader = renderer->NewShader(
        "Shaders/Basic/single_color_vertex.glsl",
        "Shaders/Basic/single_color_fragment.glsl",
        "axes"
    );

    normals_shader = renderer->NewShader(
        "Shaders/Normals/vertex.glsl",
        "Shaders/Normals/fragment.glsl",
        "Shaders/Normals/geometry.glsl",
        "normals"
    );

    curve_cp.clear();
    for (int i = 0; i < 4; i++) {
        curve_cp.push_back({ 1.f + 0.5 * i, 0.f, 2 * i });
    }

    this->BuildGrid();
    this->BuildAxes();
    this->BuildCurve();
    this->BuildMesh();

    std::cout << "mesh built, size: " << mesh.size() << std::endl;

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, this->mesh.size() * sizeof(BasicVertex), &this->mesh[0], GL_STREAM_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BasicVertex), (GLvoid*)offsetof(BasicVertex, m_Position));
    glEnableVertexAttribArray(0);

    glGenVertexArrays(1, &m_GridVAO);
    glGenBuffers(1, &m_GridVBO);
    glBindVertexArray(m_GridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_GridVBO);
    glBufferData(GL_ARRAY_BUFFER, this->grid.size() * sizeof(BasicVertex), &this->grid[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BasicVertex), (GLvoid*)offsetof(BasicVertex, m_Position));
    glEnableVertexAttribArray(0);

    glGenVertexArrays(1, &m_AxesVAO);
    glGenBuffers(1, &m_AxesVBO);
    glBindVertexArray(m_AxesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_AxesVBO);
    glBufferData(GL_ARRAY_BUFFER, this->axes.size() * sizeof(ColoredVertex), &this->axes[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (GLvoid*)offsetof(ColoredVertex, m_Position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (GLvoid*)offsetof(ColoredVertex, m_Color));
    glEnableVertexAttribArray(1);


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

    glm::mat4 model(1.f);

    points_shader->use();
    points_shader->setMat4("view", camera->GetViewMatrix());
    points_shader->setMat4("projection", projection);
    points_shader->setMat4("model", model);
    points_shader->setVec3("lightColor", glm::vec3(0.2f, 0.2f, 0.2f));
    glBindVertexArray(m_GridVAO);
    glDrawArrays(GL_LINES, 0, grid.size());
    glBindVertexArray(0);


    axes_shader->use();
    axes_shader->setMat4("view", camera->GetViewMatrix());
    axes_shader->setMat4("projection", projection);
    axes_shader->setMat4("model", model);
    glBindVertexArray(m_AxesVAO);
    glLineWidth(2);
    glDrawArrays(GL_LINES, 0, axes.size());
    glLineWidth(1);
    glBindVertexArray(0);

    light_box_shader->use();
    light_box_shader->setMat4("view", camera->GetViewMatrix());
    light_box_shader->setMat4("projection", projection);
    light_box_shader->setMat4("model", glm::mat4(1.f));
    light_box_shader->setVec3("lightColor", glm::vec3(0.5, 0.1, 0.1));
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, this->mesh.size() * sizeof(BasicVertex), &this->mesh[0], GL_STREAM_DRAW);
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    glDrawArrays(GL_TRIANGLES, 0, this->mesh.size());

    if (normals) {
        normals_shader->use();
        normals_shader->setMat4("view", camera->GetViewMatrix());
        normals_shader->setMat4("projection", projection);
        normals_shader->setMat4("model", glm::mat4(1.f));
        glDrawArrays(GL_TRIANGLES, 0, this->mesh.size());
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(0);

    points_shader->use();
    for (auto& vertex : curve_cp) {
        model = glm::mat4(1.0f);
        // model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
        model = glm::translate(model, vertex);
        model = glm::scale(model, glm::vec3(0.055f));
        points_shader->setMat4("model", model);
        points_shader->setVec3("lightColor", glm::vec3(0.8f, 0.6f, 0.f));
        cube.Render(points_shader);
    }

    if (ImGui::Begin("Settings")) {
        unsigned int index = 0;
        ImGui::Checkbox("Wireframe", &wireframe);
        ImGui::Checkbox("Normals", &normals);
        if (ImGui::DragFloat("Mesh Step Count", &source_step, 1.f, 1.f, 20.f)) {
            this->BuildMesh();
        }
        if (ImGui::DragFloat("Curve Step Count", &curve_step, 0.01, 0.05f, 0.3f)) {
            this->BuildCurve();
            this->BuildMesh();
        }
        if (ImGui::CollapsingHeader("Curve Control Points")) {
            for (auto& vert : curve_cp) {
                if (ImGui::DragFloat3(std::to_string(index++).c_str(), &vert.x, 0.1, -10.f, 10.f)) {
                    this->BuildCurve();
                    this->BuildMesh();
                }
            }
        }
        ImGui::End();
    }
    else {
        ImGui::End();
    }
}


void Lab1Scene::BuildMesh() {
    mesh.clear();

    int current_vert_line = 0;
    for (auto& vert : source_verts) {
        for (int i = 0; i < 360; i += source_step) {
            int next_vert_line = source_verts.size() > (current_vert_line + 1) ? current_vert_line + 1 : current_vert_line;

            glm::vec3 rotated_vert = glm::rotate(vert, glm::radians(float(i)), {0.f, 0.f, 1.f});
            glm::vec3 next_rotated_vert = glm::rotate(vert, glm::radians(float(i + source_step)), { 0.f, 0.f, 1.f });
            glm::vec3 next_line_vert = glm::rotate(source_verts[next_vert_line], glm::radians(float(i)), { 0.f, 0.f, 1.f });
            glm::vec3 next_line_vert_2 = glm::rotate(source_verts[next_vert_line], glm::radians(float(i + source_step)), { 0.f, 0.f, 1.f });

            mesh.push_back(BasicVertex(rotated_vert));
            mesh.push_back(BasicVertex(next_rotated_vert));
            mesh.push_back(BasicVertex(next_line_vert));


            mesh.push_back(BasicVertex(next_line_vert));
            mesh.push_back(BasicVertex(next_line_vert_2));
            mesh.push_back(BasicVertex(next_rotated_vert));
        }
        current_vert_line++;
    }
}


void Lab1Scene::BuildGrid() {
    // x axis
    grid.clear();
    for (int i = -10; i <= 10; i++) {
        grid.push_back(BasicVertex{ glm::vec3(i, 0.f, -10.f) });
        if (i == 0) [[unlikely]] {
            grid.push_back(BasicVertex{glm::vec3(i, 0.f, 0.f)});
        }
        else [[likely]] {
            grid.push_back(BasicVertex{glm::vec3(i, 0.f, 10.f)});
        }
    }

    // y axis
    for (int i = -10; i <= 10; i++) {
        grid.push_back(BasicVertex{ glm::vec3(-10.f, 0.f, i) });
        if (i == 0) [[unlikely]] {
            grid.push_back(BasicVertex{ glm::vec3(0.f, 0.f, i) });
        }
        else [[likely]] {
            grid.push_back(BasicVertex{ glm::vec3(10.f, 0.f, i) });
        }
    }
}


void Lab1Scene::BuildAxes() {
    // x axis
    axes.clear();
    axes.push_back(ColoredVertex{ glm::vec3(0.f, 0.f, 0.01f), glm::vec3(1.f, 0.f, 0.f) });
    axes.push_back(ColoredVertex{ glm::vec3(10.f, 0.f, 0.01f), glm::vec3(1.f, 0.f, 0.f) });

    // y axis
    axes.push_back(ColoredVertex{ glm::vec3(0.f, 0.f, 0.01f), glm::vec3(0.f, 1.f, 0.f) });
    axes.push_back(ColoredVertex{ glm::vec3(0.f, 10.f, 0.01f), glm::vec3(0.f, 1.f, 0.f) });

    // z axis
    axes.push_back(ColoredVertex{ glm::vec3(0.f, 0.f, 0.01f), glm::vec3(0.f, 0.f, 1.f) });
    axes.push_back(ColoredVertex{ glm::vec3(0.f, 0.f, 10.01f), glm::vec3(0.f, 0.f, 1.f) });
}


void Lab1Scene::BuildCurve() {
    source_verts.clear();
    for (float t = 0.f; t < 1.f; t += curve_step) {
        float t3 = t * t * t;
        float t2 = t * t;
        float _t3 = pow((1 - t), 3);
        float _t2 = pow((1 - t), 2);
        source_verts.push_back({
            _t3 * curve_cp[0].x + 3 * t * _t2 * curve_cp[1].x + 3 * t2 * (1 - t) * curve_cp[2].x + t3 * curve_cp[3].x,
            _t3 * curve_cp[0].y + 3 * t * _t2 * curve_cp[1].y + 3 * t2 * (1 - t) * curve_cp[2].y + t3 * curve_cp[3].y,
            _t3 * curve_cp[0].z + 3 * t * _t2 * curve_cp[1].z + 3 * t2 * (1 - t) * curve_cp[2].z + t3 * curve_cp[3].z,
        });
    }
}