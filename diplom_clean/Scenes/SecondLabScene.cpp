#include "SecondLabScene.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <imgui/imgui.h>

#include <random>

#include "../Renderer/Renderer.h"
#include "../ImGui/ImGuiLogger.h"


Lab2Scene::~Lab2Scene() {
    glDeleteFramebuffers(1, &plot_fbo);
    glDeleteTextures(1, &plot_texture);

    unsigned int vaos[4] = { m_VAO, m_GridVAO, m_AxesVAO, m_PlotVAO };
    unsigned int vbos[4] = { m_VBO, m_GridVBO, m_AxesVBO, m_PlotVBO };
    glDeleteVertexArrays(4, vaos);
    glDeleteBuffers(4, vbos);
}

void Lab2Scene::Setup() {
    Logger* logger = &Logger::instance();
    logger->AddLog("[Lab1] setup started...\n");
    Renderer* renderer = &Renderer::instance();

    translation = glm::vec3(0.f);

    auto arcball_camera = renderer->NewCamera({ 0.0f, 0.0f, 3.0f }, "arcball_camera", Camera::Camera_Type::ARCBALL);
    renderer->SetActiveCamera("arcball_camera");

    tess_shader = renderer->NewShader(
        "Shaders/Tessellation/tessellation_vertex.glsl", 
        "Shaders/Tessellation/tessellation_fragment.glsl", 
        "Shaders/Tessellation/tessellation_control.glsl", 
        "Shaders/Tessellation/tessellation_evaluation.glsl", 
        "l_box"
    );

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

    plot_shader = renderer->NewShader(
        "Shaders/Basic/vertex.glsl",
        "Shaders/Basic/fragment.glsl",
        "plot"
    );

    // define shape here

    this->BuildMesh();
    this->BuildGrid();
    this->BuildAxes();
    this->BuildPlot();

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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


    glGenVertexArrays(1, &m_PlotVAO);
    glGenBuffers(1, &m_PlotVBO);
    glBindVertexArray(m_PlotVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_PlotVBO);
    glBufferData(GL_ARRAY_BUFFER, this->plot.size() * sizeof(ColoredVertex), &this->plot[0], GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (GLvoid*)offsetof(ColoredVertex, m_Position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (GLvoid*)offsetof(ColoredVertex, m_Color));
    glEnableVertexAttribArray(1);

    glGenFramebuffers(1, &plot_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, plot_fbo);

    glGenTextures(1, &plot_texture);
    glBindTexture(GL_TEXTURE_2D, plot_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Rendering::SCREEN_WIDTH, Rendering::SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, plot_texture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

    // render plot into framebuffer texture and project it on plane
    // grid in default vao, with gray quad behind it
    // coord lines in default vao

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glBindFramebuffer(GL_FRAMEBUFFER, plot_fbo);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(m_PlotVAO);
    plot_shader->use();
    glLineWidth(3);
    glDrawArrays(GL_LINE_STRIP, 0, plot.size());
    glLineWidth(1);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


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

    model = glm::mat4(1.f);
    model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
    model = glm::translate(model, translation);

    tess_shader->use();
    tess_shader->setMat4("view", camera->GetViewMatrix());
    tess_shader->setMat4("projection", projection);
    tess_shader->setMat4("model", model);
    tess_shader->setInt("inner", tess_inner);
    tess_shader->setInt("outer", tess_outer);
    tess_shader->setFloat("time", glfwGetTime());
    tess_shader->setInt("image", 0);
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, this->mesh.size() * sizeof(BasicVertex), &this->mesh[0], GL_STREAM_DRAW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, plot_texture);
    glDrawArrays(GL_PATCHES, 0, this->mesh.size());

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    points_shader->use();
    points_shader->setMat4("view", camera->GetViewMatrix());
    points_shader->setMat4("projection", projection);

    for (auto& vertex : mesh) {
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
        model = glm::translate(model, vertex.m_Position + translation);
        model = glm::scale(model, glm::vec3(0.055f));
        points_shader->setMat4("model", model);
        points_shader->setVec3("lightColor", glm::vec3(0.8f, 0.6f, 0.f));
        cube.Render((*points_shader));
    }

    if (ImGui::Begin("Controls")) [[likely]] {
        ImGui::DragFloat3("Translation", &translation.x, 0.1, -100.f, 100.f);
        ImGui::InputInt("tess outer", &tess_outer);
        ImGui::InputInt("tess inner", &tess_inner);
        if (ImGui::Checkbox("Wireframe", &wireframe)) {
            glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        }

        if (ImGui::CollapsingHeader("Points")) {
            unsigned int index = 0;
            for (auto& vertex : mesh) {
                ImGui::DragFloat3(std::to_string(index++).c_str(), &vertex.m_Position.x, 0.1, -100.f, 100.f);
                if (index % 4 == 0) ImGui::Separator();
            }
        }
        if (ImGui::CollapsingHeader("Plot image")) {
            const auto size = ImVec2(Rendering::SCREEN_WIDTH / 5.0, Rendering::SCREEN_HEIGHT / 5.0);
            ImGui::Image((ImTextureID)plot_texture, size, ImVec2(0, 1), ImVec2(1, 0));
        }
        if (ImGui::CollapsingHeader("Plot controls")) {
            ImGui::InputInt("R", &m_PlotR);
            ImGui::InputInt("r", &m_Plot_r);
            ImGui::DragFloat("T step", &m_Tstep, 0.1, 0.2, 5.0);
            ImGui::DragFloat("T min", &m_Tmin, 1, 0.0, 90.0);
            ImGui::DragFloat("T max", &m_Tmax, 1, 180.0, 1800.0);
            if (ImGui::Button("Rebuild")) {
                this->BuildPlot();
                glBindVertexArray(m_PlotVAO);
                glBindBuffer(GL_ARRAY_BUFFER, m_PlotVBO);
                glBufferData(GL_ARRAY_BUFFER, this->plot.size() * sizeof(ColoredVertex), &this->plot[0], GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
            }
        }

        ImGui::End();
    }
    else {
        ImGui::End();
    }

    glBindVertexArray(0);
}


void Lab2Scene::BuildMesh() {
    mesh.clear();

    mesh.push_back(BasicVertex{glm::vec3(-2.0f, -0.1f, 1.0f), });
    mesh.push_back(BasicVertex{glm::vec3(-1.0f, -2.0f, 1.0f), });
    mesh.push_back(BasicVertex{glm::vec3(1.0f, -2.0f, 1.0f),  });
    mesh.push_back(BasicVertex{glm::vec3(2.0f, -2.0f, 1.0f),  });

    mesh.push_back(BasicVertex{glm::vec3(-2.0f, -0.f, 1.0f),  });
    mesh.push_back(BasicVertex{glm::vec3(-1.0f, -1.f, 0.0f),  });
    mesh.push_back(BasicVertex{glm::vec3(1.0f, -1.0f, -0.5),  });
    mesh.push_back(BasicVertex{glm::vec3(2.0f, -1.f, -0.5f),  });

    mesh.push_back(BasicVertex{glm::vec3(-2.0f, 0.f, 1.0f),   });
    mesh.push_back(BasicVertex{glm::vec3(-1.0f, 1.f, 0.0f),   });
    mesh.push_back(BasicVertex{glm::vec3(1.0f, 1.f, -0.5f),   });
    mesh.push_back(BasicVertex{glm::vec3(2.0f, 1.f, -0.5f),   });
                                                              
    mesh.push_back(BasicVertex{glm::vec3(-2.0f, 0.1f, 1.0f),  });
    mesh.push_back(BasicVertex{glm::vec3(-1.f, 2.0f, 1.0f),   });
    mesh.push_back(BasicVertex{glm::vec3(1.0f, 2.0f, 1.0f),   });
    mesh.push_back(BasicVertex{glm::vec3(2.0f, 2.0f, 1.0f),   });
}


void Lab2Scene::BuildGrid() {
    // x axis
    for (int i = -10; i <= 10; i++) {
        grid.push_back(BasicVertex{glm::vec3(i, 0.f, -10.f)});
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


void Lab2Scene::BuildAxes() {
    // x axis
    axes.push_back(ColoredVertex{ glm::vec3(0.f, 0.f, 0.01f), glm::vec3(1.f, 0.f, 0.f) });
    axes.push_back(ColoredVertex{ glm::vec3(10.f, 0.f, 0.01f), glm::vec3(1.f, 0.f, 0.f) });

    // y axis
    axes.push_back(ColoredVertex{ glm::vec3(0.f, 0.f, 0.01f), glm::vec3(0.f, 0.f, 1.f) });
    axes.push_back(ColoredVertex{ glm::vec3(0.f, 10.f, 0.01f), glm::vec3(0.f, 0.f, 1.f) });

    // z axis
    axes.push_back(ColoredVertex{ glm::vec3(0.f, 0.f, 0.01f), glm::vec3(0.f, 1.f, 0.f) });
    axes.push_back(ColoredVertex{ glm::vec3(0.f, 0.f, 10.01f), glm::vec3(0.f, 1.f, 0.f) });
}

double deg2rad(double deg) {
    return deg * 3.14 / 180.0;
}

float GetParametricX(float& R, float& r, float& t, float& k) {
    return r * (k - 1.) * cos(deg2rad(t)) + r * cos(deg2rad((k - 1.) * t));
}

float GetParametricY(float& R, float& r, float& t, float& k) {
    return r * (k - 1.) * sin(deg2rad(t)) - r * sin(deg2rad((k - 1.) * t));
}


void Lab2Scene::BuildPlot() {
    float R = m_PlotR;
    float r = m_Plot_r;
    float k = (float)R / (float)r;
    float delta = R - r;

    plot.clear();
    for (float t = m_Tmin; t < m_Tmax; t += m_Tstep) {

        float new_x = GetParametricX(R, r, t, k) / R;
        float new_y = GetParametricY(R, r, t, k) / R;

        plot.push_back(ColoredVertex{
            glm::vec3(new_x, new_y, 0.f), // pos
            glm::vec3(1.f, 0.f, 0.f), // color
        });
    }
}