#include "InstancingScene.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <imgui/imgui.h>
#include <random>

#include "../Components/Transform.h"
#include "../Components/Light.h"
#include "../Components/Render.h"
#include <entt/entt.hpp>

#include "../Renderer/Renderer.h"
#include "../ImGui/ImGuiLogger.h"

InstancingScene::~InstancingScene() {
    glDeleteFramebuffers(2, m_PingPongFBO);
    glDeleteTextures(2, m_PingPongColorBuffers);
    glDeleteTextures(2, m_hdrColorBuffers);

    glDeleteBuffers(1, &m_SSBO);
}

void InstancingScene::Setup() {
    Logger* logger = &Logger::instance();
    logger->AddLog("[Scene] setup started...\n");
    Renderer* renderer = &Renderer::instance();

    renderer->m_Settings._compress_textures = false;
    InstancedModel::_enable_frustum = false;

    auto arcball_camera = renderer->NewCamera({ 0.0f, 0.0f, 3.0f }, "arcball_camera", Camera::Camera_Type::ARCBALL);
    auto fly_camera = renderer->NewCamera({ 0.0f, 0.0f, 3.0f }, "fly_camera", Camera::Camera_Type::FLYCAM);
    renderer->SetActiveCamera("arcball_camera");

    auto brick_tex = renderer->NewTexture(
        "Resource/textures/brickwall.jpg", "brickwall", "texture_diffuse", false
    );
    renderer->NewTexture(
        "Resource/textures/brickwall_normal.jpg", "brickwall_normal", "texture_normal", false
    );

    renderer->NewTexture(
        "Resource/textures/brickwall_specular.jpg", "brickwall_specular", "texture_specular", false
    );

    geometry_pass_shader = renderer->NewShader(
        "Shaders/Deferred/geometry_pass_vertex.glsl", "Shaders/Deferred/geometry_pass_fragment_2.glsl", "g_pass");
    instanced_gp_shader = renderer->NewShader(
        "Shaders/Deferred/geometry_instanced_vertex.glsl", "Shaders/Deferred/geometry_instanced_fragment_2.glsl", "instanced_g_pass");
    blur_shader = renderer->NewShader(
        "Shaders/Deferred/blur_vertex.glsl", "Shaders/Deferred/blur_fragment.glsl", "blur_shader");
    final_shader = renderer->NewShader(
        "Shaders/Deferred/bloom_final_vertex.glsl", "Shaders/Deferred/bloom_final_fragment.glsl", "bloom_shader");

    auto quad = renderer->NewQuad("brickwall", "g_pass");
    quad->SetNormalMap("brickwall_normal");
    quad->SetRotation(-90, { 1.f, 0.f, 0.f });
    quad->SetScale({ 20.f, 20.f, 20.0 });
    quad->SetTransform({ 0.f, -1.f, 0.f });
    quad->SetUV(
        { 0.0f, 10.0f },
        { 0.0f, 0.0f },
        { 10.0f, 0.0f },
        { 10.0f, 10.0f }
    );

    m_Registry.on_construct<InstancedModelComponent>().connect<&InstancingScene::AddInstancedModel>(this);
    m_Registry.on_update<TransformComponent>().connect<&InstancingScene::UpdateInstancedModel>(this);

    for (int i = 0; i < m_ModelsStride; i++) {
        for (int j = 0; j < m_ModelsStride; j++) {
            auto entity = AddEntity();
            m_Registry.emplace<TransformComponent>(entity,
                glm::vec3{ i * 5.f - m_ModelsStride * 2.5f, 1.f, j * 5.f - m_ModelsStride * 2.5f }, glm::vec3{ 0.7f , 0.7f , 0.7f });
            m_Registry.emplace<InstancedModelComponent>(entity, "Models/backpack/backpack.obj", "instanced_g_pass");
        }
    }

    int SCR_WIDTH = Rendering::SCREEN_WIDTH;
    int SCR_HEIGHT = Rendering::SCREEN_HEIGHT;

    glGenFramebuffers(1, &m_FinalFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FinalFBO);
    m_FinalTexture.Set(SCR_WIDTH, SCR_HEIGHT, GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenBuffers(1, &m_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO);
    std::vector<LightSource> m_LightData;
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Rendering::LightData) * m_LightData.size(), &m_LightData[0], GL_STREAM_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenFramebuffers(1, &m_hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);

    glGenTextures(2, m_hdrColorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, m_hdrColorBuffers[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_hdrColorBuffers[i], 0
        );
    }
    unsigned int bloom_attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, bloom_attachments);

    unsigned int rboDepth2;
    glGenRenderbuffers(1, &rboDepth2);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth2);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Rendering::SCREEN_WIDTH, Rendering::SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth2);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(2, m_PingPongFBO);
    glGenTextures(2, m_PingPongColorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_PingPongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, m_PingPongColorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_PingPongColorBuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    blur_shader->use();
    blur_shader->setInt("image", 0);
    final_shader->use();
    final_shader->setInt("scene", 0);
    final_shader->setInt("bloomBlur", 1);

    BuildLightData();

    auto skyboxVertices = GetSkyboxVerts();

    glGenVertexArrays(1, &m_SkyboxVAO);
    glGenBuffers(1, &m_SkyboxVBO);
    glBindVertexArray(m_SkyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_SkyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * skyboxVertices.size(), &skyboxVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    logger->AddLog("[Scene] setup finished.\n");
}

void InstancingScene::Render() {
    Renderer* renderer = &Renderer::instance();
    Logger* logger = &Logger::instance();
    static bool keep_open = true;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(
        glm::radians(renderer->m_CurrentCamera->m_Zoom),
        (float)Rendering::SCREEN_WIDTH / (float)Rendering::SCREEN_HEIGHT,
        0.1f, 100.0f
    );
    glm::mat4 view = renderer->m_CurrentCamera->GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = projection * view * model;

    std::vector<Rendering::LightData> temporary_light_data;

    glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto light_view = m_Registry.view<const Rendering::LightData>();
        size_t index = 0;
        for (const auto& [entity, light_data] : light_view.each()) {
            if (index++ >= renderer->m_Settings.current_light_limits) break;
            // if (IsVisibleFrom(light_data.Position, mvp, light_data.Radius)) {
                temporary_light_data.push_back(light_data);
            // }
        }
        this->RegenerateLights(temporary_light_data);

        renderer->m_VisibleLights = temporary_light_data.size();
        geometry_pass_shader->use();
        geometry_pass_shader->setMat4("projection", projection);
        geometry_pass_shader->setMat4("view", view);
        geometry_pass_shader->setInt("light_count", (int)temporary_light_data.size());
        geometry_pass_shader->setFloat("intensity", renderer->m_Settings.intensity);
        geometry_pass_shader->setFloat("bloom_threshold", renderer->m_Settings.bloom_threshold);
        geometry_pass_shader->setFloat("ambient", renderer->m_Settings.ambient);
        geometry_pass_shader->setVec3("viewPos", renderer->m_CurrentCamera->m_Position);

        instanced_gp_shader->use();
        instanced_gp_shader->setInt("light_count", (int)temporary_light_data.size());
        instanced_gp_shader->setFloat("intensity", renderer->m_Settings.intensity);
        instanced_gp_shader->setFloat("bloom_threshold", renderer->m_Settings.bloom_threshold);
        instanced_gp_shader->setFloat("ambient", renderer->m_Settings.ambient);
        instanced_gp_shader->setVec3("viewPos", renderer->m_CurrentCamera->m_Position);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Rendering::LightData) * temporary_light_data.size(), &temporary_light_data[0], GL_STREAM_DRAW);

        glPolygonMode(GL_FRONT_AND_BACK, renderer->m_Settings.wireframe ? GL_LINE : GL_FILL);
        renderer->Render(m_Registry);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    bool horizontal = true, first_iteration = true;
    {
        blur_shader->use();
        for (size_t i = 0; i < renderer->m_Settings.bloom_radius; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_PingPongFBO[horizontal]);
            blur_shader->setInt("horizontal", horizontal);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? m_hdrColorBuffers[1] : m_PingPongColorBuffers[!horizontal]);
            renderer->SimpleQuad();
            horizontal = !horizontal;

            first_iteration = false;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_FinalFBO);
    {
        final_shader->use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_hdrColorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_PingPongColorBuffers[!horizontal]);
        final_shader->setInt("bloom", true);
        final_shader->setFloat("exposure", renderer->m_Settings.exposure);
        renderer->SimpleQuad();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ImGui editor pass
    {
        renderer->Dockspace();

        if (ImGui::Begin(U8_CAST("Головне вікно"), &keep_open)) {
            ImVec2 avail_size = ImGui::GetContentRegionAvail();
            ImVec2 size(avail_size.x, avail_size.x * 9.0f / 16.0f);
            ImGui::Image((ImTextureID)(uint64_t)m_FinalTexture.m_Id, size, ImVec2(-1, 1), ImVec2(0, 0));

            m_Active = ImGui::IsItemHovered();

            renderer->PerfCounter();

            ImGui::End();
        }

        renderer->GatherImGui();

        if (ImGui::Begin(U8_CAST("Буфери"))) {
            ImVec2 avail_size = ImGui::GetContentRegionAvail();
            ImVec2 size(avail_size.x, avail_size.x * 9.0f / 16.0f);

            ImGui::Image((ImTextureID)(uint64_t)m_hdrColorBuffers[0], size, ImVec2(0, 1), ImVec2(1, 0));
            ImGui::Image((ImTextureID)(uint64_t)m_hdrColorBuffers[1], size, ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::End();
    }
}