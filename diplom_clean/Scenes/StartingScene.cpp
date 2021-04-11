#include "StartingScene.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <imgui/imgui.h>
#include <random>

#include "scene_util.h"
#include "../Components/Transform.h"
#include "../Components/Light.h"
#include "../Components/Render.h"
#include <entt/entt.hpp>

#include "../Renderer/Renderer.h"
#include "../ImGui/ImGuiLogger.h"


bool IsVisibleFrom(glm::vec4 position, glm::mat4& mvp, float radius) {
    glm::vec4 worldspace = mvp * position;
    return abs(worldspace.x) < worldspace.w + radius &&
        abs(worldspace.y) < worldspace.w + radius &&
        0 < worldspace.z &&
        abs(worldspace.z) < worldspace.w + radius;
}


StartingScene::~StartingScene() {
    glDeleteFramebuffers(1, &m_GeometryFBO);

    glDeleteFramebuffers(2, m_PingPongFBO);
    glDeleteTextures(2, m_PingPongColorBuffers);
    glDeleteTextures(2, m_hdrColorBuffers);

    glDeleteBuffers(1, &m_SSBO);

    glDeleteRenderbuffers(1, &m_DepthRBO);

}

entt::entity StartingScene::AddEntity() {
    return registry.create();
}

void StartingScene::RemoveEntity(entt::entity& ent) {
    registry.destroy(ent);
}

void StartingScene::Setup() {
    Logger* logger = &Logger::instance();
    logger->AddLog("[Scene] setup started...\n");
    Renderer* renderer = &Renderer::instance();

    if (glfwExtensionSupported("GL_EXT_texture_compression_s3tc"))
    {
        std::cout << "s3tc compression available" << std::endl;
    }
    else {
        std::cout << "s3tc compression NOT AVAILABLE" << std::endl;
    }

    auto arcball_camera = renderer->NewCamera({ 0.0f, 0.0f, 3.0f }, "arcball_camera", Camera::Camera_Type::ARCBALL);
    auto fly_camera = renderer->NewCamera({ 0.0f, 0.0f, 3.0f }, "fly_camera", Camera::Camera_Type::FLYCAM);
    renderer->SetActiveCamera("arcball_camera");

    auto brick_tex = renderer->NewTexture(
        "Resource/textures/brickwall.jpg", "brickwall", "texture_diffuse"
    );
    renderer->NewTexture(
        "Resource/textures/brickwall_normal.jpg", "brickwall_normal", "texture_normal", false
    );
    renderer->NewTexture(
        "Resource/textures/brickwall_specular.jpg", "brickwall_specular", "texture_specular"
    );

    auto gatari_tex = renderer->NewTexture(
        "Resource/textures/blending_transparent_window.png", "gatari", "texture_diffuse"
    );
    simple_tex_shader = renderer->NewShader(
        "Shaders/Basic/vertex.glsl", "Shaders/Basic/fragment.glsl", "simple_shader");

    simple_g_shader = renderer->NewShader(
        "Shaders/Deferred/geometry_pass_vertex.glsl", "Shaders/Deferred/geometry_pass_fragment_simple.glsl", "geometry_simple_shader");

    geometry_pass_shader = renderer->NewShader(
        "Shaders/Deferred/geometry_pass_vertex.glsl", "Shaders/Deferred/geometry_pass_fragment.glsl", "g_pass");
    instanced_gp_shader = renderer->NewShader(
        "Shaders/Deferred/geometry_instanced_vertex.glsl", "Shaders/Deferred/geometry_pass_fragment.glsl", "instanced_g_pass");
    lighting_pass_shader = renderer->NewShader(
        "Shaders/Deferred/lighting_pass_vertex.glsl", "Shaders/Deferred/lighting_pass_fragment.glsl", "l_pass");
    light_box_shader = renderer->NewShader(
        "Shaders/Deferred/light_box_vertex.glsl", "Shaders/Deferred/light_box_fragment.glsl", "l_box");
    blur_shader = renderer->NewShader(
        "Shaders/Deferred/blur_vertex.glsl", "Shaders/Deferred/blur_fragment.glsl", "blur_shader");
    final_shader = renderer->NewShader(
        "Shaders/Deferred/bloom_final_vertex.glsl", "Shaders/Deferred/bloom_final_fragment.glsl", "bloom_shader");
    skybox_shader = renderer->NewShader(
        "Shaders/Basic/skybox_vertex.glsl", "Shaders/Basic/skybox_fragment.glsl", "skybox_shader");

    auto quad = renderer->NewQuad("brickwall", "g_pass");
    quad->SetNormalMap("brickwall_normal");
    quad->SetRotation(-90, { 1.f, 0.f, 0.f });
    quad->SetScale({ 20.f, 20.f, 20.0 });
    quad->SetTransform({ 0.f, -1.f, 0.f });

    m_Quad = std::shared_ptr<Quad>(new Quad(gatari_tex, simple_tex_shader));
    m_Quad->SetNormalMap("brickwall_normal");
    m_Quad->SetRotation(0.f, { 1.f, 0.f, 0.f });
    m_Quad->SetScale({ 20.f, 4.f, 4.f });
    m_Quad->SetTransform({ 0.f, 2.f, 1.f });

    registry.on_construct<InstancedModelComponent>().connect<&StartingScene::AddInstancedModel>(this);
    registry.on_update<TransformComponent>().connect<&StartingScene::UpdateInstancedModel>(this);

    for (int i = -m_ModelsStride; i <= m_ModelsStride; i++) {
        for (int j = -m_ModelsStride; j <= m_ModelsStride; j++) {

            auto entity = AddEntity();
            registry.emplace<TransformComponent>(entity, glm::vec3{ i * 5.f, 1.f, j * 5.f }, glm::vec3{ 0.7f , 0.7f , 0.7f });
            // registry.emplace<InstancedModelComponent>(entity, "Models/backpack/backpack.obj", "instanced_g_pass");
            registry.emplace<InstancedModelComponent>(entity, "Models/backpack/backpack.obj", "instanced_g_pass");
        }
    }

    skybox = renderer->NewCubemap({
        "Resource/skybox/elyvisions/tron_lf.png",
        "Resource/skybox/elyvisions/tron_rt.png",
        "Resource/skybox/elyvisions/tron_up.png",
        "Resource/skybox/elyvisions/tron_dn.png",
        "Resource/skybox/elyvisions/tron_bk.png",
        "Resource/skybox/elyvisions/tron_ft.png",
    });

    int SCR_WIDTH = Rendering::SCREEN_WIDTH;
    int SCR_HEIGHT = Rendering::SCREEN_HEIGHT;
    
    glGenFramebuffers(1, &m_GeometryFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_GeometryFBO);
    
    m_GeometryPosition.Set(SCR_WIDTH, SCR_HEIGHT, GL_COLOR_ATTACHMENT0);
    m_GeometryNormals.Set(SCR_WIDTH, SCR_HEIGHT, GL_COLOR_ATTACHMENT1);
    m_GeometryAlbedoSpec.Set(SCR_WIDTH, SCR_HEIGHT, GL_COLOR_ATTACHMENT2);

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    glGenRenderbuffers(1, &m_DepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Rendering::SCREEN_WIDTH, Rendering::SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    glGenFramebuffers(1, &m_FinalFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FinalFBO);
    m_FinalTexture.Set(SCR_WIDTH, SCR_HEIGHT, GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glGenBuffers(1, &m_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO);
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

    lighting_pass_shader->use();
    lighting_pass_shader->setInt("gPosition", 0);
    lighting_pass_shader->setInt("gNormal", 1);
    lighting_pass_shader->setInt("gAlbedoSpec", 2);
    blur_shader->use();
    blur_shader->setInt("image", 0);
    final_shader->use();
    final_shader->setInt("scene", 0);
    final_shader->setInt("bloomBlur", 1);

    BuildLightData();

    auto skyboxVertices= GetSkyboxVerts();

    glGenVertexArrays(1, &m_SkyboxVAO);
    glGenBuffers(1, &m_SkyboxVBO);
    glBindVertexArray(m_SkyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_SkyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * skyboxVertices.size(), &skyboxVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    logger->AddLog("[Scene] setup finished.\n");
}

void StartingScene::Render() {
    Renderer* renderer = &Renderer::instance();
    Logger* logger = &Logger::instance();
    static bool keep_open = true;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, m_GeometryFBO);
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

    auto light_view = registry.view<const Rendering::LightData>();
    for(const auto& [entity, light_data]: light_view.each()) {
        if (IsVisibleFrom(light_data.Position, mvp, light_data.Radius)) {
            temporary_light_data.push_back(light_data);
        }
    }
    this->RegenerateLights(temporary_light_data);

    renderer->m_VisibleLights = temporary_light_data.size();
    geometry_pass_shader->use();
    geometry_pass_shader->setMat4("projection", projection);
    geometry_pass_shader->setMat4("view", view);
    glPolygonMode(GL_FRONT_AND_BACK, renderer->m_Settings.wireframe ? GL_LINE : GL_FILL);
    renderer->Render(registry);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    simple_g_shader->use();
    simple_g_shader->setMat4("projection", projection);
    simple_g_shader->setMat4("view", view);

    for (unsigned int i = 0; i < temporary_light_data.size(); i++)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(temporary_light_data[i].Position));
        model = glm::scale(model, glm::vec3(0.055f));
        simple_g_shader->setMat4("model", model);
        simple_g_shader->setVec3("lightColor", glm::vec3(temporary_light_data[i].Color));
        m_CubeModel.Render(simple_g_shader);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_Quad->Render(renderer->m_CurrentCamera, projection);
    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    lighting_pass_shader->use();

    m_GeometryPosition.Use(GL_TEXTURE0);
    m_GeometryNormals.Use(GL_TEXTURE1);
    m_GeometryAlbedoSpec.Use(GL_TEXTURE2);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Rendering::LightData) * temporary_light_data.size(), &temporary_light_data[0], GL_STREAM_DRAW);

    glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    lighting_pass_shader->setInt("light_count", temporary_light_data.size());
    lighting_pass_shader->setFloat("intensity", renderer->m_Settings.intensity);
    lighting_pass_shader->setFloat("bloom_threshold", renderer->m_Settings.bloom_threshold);
    lighting_pass_shader->setFloat("ambient", renderer->m_Settings.ambient);
    lighting_pass_shader->setVec3("viewPos", renderer->m_CurrentCamera->m_Position);
    renderer->SimpleQuad();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    m_GeometryPosition.Reset(GL_TEXTURE0);
    m_GeometryNormals.Reset(GL_TEXTURE1);
    m_GeometryAlbedoSpec.Reset(GL_TEXTURE2);

    // copy depth to correcly handle light source cubes
    /*
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GeometryFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_hdrFBO);
    glBlitFramebuffer(0, 0, 
        Rendering::SCREEN_WIDTH, Rendering::SCREEN_HEIGHT, 0, 0, 
        Rendering::SCREEN_WIDTH, Rendering::SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);
    */
    if (renderer->m_Settings.skybox) {
        glDepthFunc(GL_LEQUAL);
        glBindVertexArray(m_SkyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->id);
        skybox_shader->use();
        glm::mat4 t_view = glm::mat4(glm::mat3(view));
        skybox_shader->setMat4("view", t_view);
        skybox_shader->setMat4("projection", projection);
        skybox_shader->setInt("skybox", 0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    bool horizontal = true, first_iteration = true;
    blur_shader->use();
    for (unsigned int i = 0; i < renderer->m_Settings.bloom_radius; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_PingPongFBO[horizontal]);
        blur_shader->setInt("horizontal", horizontal);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, first_iteration ? m_hdrColorBuffers[1] : m_PingPongColorBuffers[!horizontal]);
        renderer->SimpleQuad();
        horizontal = !horizontal;

        first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_FinalFBO);
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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    Dockspace();

    if (ImGui::Begin("Main Window", &keep_open)) {
        ImVec2 avail_size = ImGui::GetContentRegionAvail();
        ImVec2 size(avail_size.x, avail_size.x * 9.0 / 16.0);
        ImGui::Image((ImTextureID)m_FinalTexture.m_Id, size, ImVec2(-1, 1), ImVec2(0, 0));

        active = ImGui::IsItemHovered();

        PerfCounter();

        ImGui::End();
    }

    renderer->GatherImGui();

    if (ImGui::Begin("Buffer View")) {
        ImVec2 avail_size = ImGui::GetContentRegionAvail();
        ImVec2 size(avail_size.x, avail_size.x * 9.0 / 16.0);
        ImGui::Image((ImTextureID)m_GeometryAlbedoSpec.m_Id, size, ImVec2(-1, 1), ImVec2(0, 0));

        ImGui::Image((ImTextureID)m_hdrColorBuffers[0], size, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::Image((ImTextureID)m_hdrColorBuffers[1], size, ImVec2(0, 1), ImVec2(1, 0));

        ImGui::Image((ImTextureID)m_PingPongColorBuffers[0], size, ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();
    if (ImGui::Begin("Renderer")) {
        if (ImGui::CollapsingHeader("Instances")) {
            int index = 0;
            for (auto&& [entity, transform, model] : registry.view<TransformComponent, InstancedModelComponent>().each()) {
                ImGui::PushID(index++);
                if (ImGui::DragFloat3("Translation", &transform.translation.x, 0.1f)) {
                    registry.patch<TransformComponent>(entity, [](auto& transform) {});
                }
                if (ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f)) {
                    registry.patch<TransformComponent>(entity, [](auto& transform) {});
                }

                if (ImGui::DragFloat("Radians", &transform.rotation_radians, 0.05f)) {
                    registry.patch<TransformComponent>(entity, [](auto& transform) {});
                }
                if (ImGui::DragFloat3("Rotation", &transform.rotation_axis.x, 0.1f)) {
                    registry.patch<TransformComponent>(entity, [](auto& transform) {});
                }
                ImGui::PopID();
                ImGui::Separator();
            }
        }
    }
    ImGui::End();
}

void StartingScene::AddInstancedModel(entt::registry& registry, entt::entity entity) {
    Renderer* renderer = &Renderer::instance();

    auto& transform = registry.get<TransformComponent>(entity);
    auto& render = registry.get<InstancedModelComponent>(entity);

    auto instance_ctrl = renderer->GetInstancedModel(render.m_ModelName);
    if (instance_ctrl == nullptr) {
        instance_ctrl = renderer->NewInstancedModel(render.m_ModelName, render.m_ShaderName, render.m_ModelName);
    }
    render.m_InstanceIdx = instance_ctrl->Add(transform.GetModelMatrix());
}

void StartingScene::UpdateInstancedModel(entt::registry& registry, entt::entity entity) {
    Renderer* renderer = &Renderer::instance();

    auto& transform = registry.get<TransformComponent>(entity);
    auto render = registry.try_get<InstancedModelComponent>(entity);

    if (render != nullptr && render->m_InstanceIdx != -1) {
        auto instance_ctrl = renderer->GetInstancedModel(render->m_ModelName);
        if (instance_ctrl != nullptr) {
            instance_ctrl->Update(render->m_InstanceIdx, transform.GetModelMatrix());
        }
    }
}

void StartingScene::RegenerateLights(std::vector<Rendering::LightData>& visible_lights) {
    Renderer* renderer = &Renderer::instance();
    const float constant = renderer->m_Settings.light_constant;
    const float linear = renderer->m_Settings.light_linear;
    const float quadratic = renderer->m_Settings.light_quadratic;
    const float intensity = renderer->m_Settings.intensity;

    for (auto& light : visible_lights) {

        light.Linear = linear;
        light.Quadratic = quadratic;
        const float maxBrightness = std::fmaxf(std::fmaxf(light.Color.r, light.Color.g), light.Color.b);
        float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - ((256.0f / 5.f) * maxBrightness)))) / (2.0f * quadratic);
        light.Radius = radius * intensity;
    }
}


void StartingScene::Dockspace() {
    static bool p_open = true;
    static bool opt_fullscreen_persistant = true;
    bool opt_fullscreen = opt_fullscreen_persistant;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
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


void StartingScene::PerfCounter() {
    const float DISTANCE = 10.0f;
    static bool open = true;
    static int corner = 1;

    Renderer* renderer = &Renderer::instance();

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
    if (ImGui::Begin("Renderer: Frame Stats", &open, window_flags))
    {
        ImGui::Text("Frame Time: %.2f ms", renderer->m_FrameTime * 1000.0);
        ImGui::Text("FPS: %.1f", 1.0 / renderer->m_FrameTime);
        ImGui::Text("Visible Light: %d", renderer->m_VisibleLights);
        ImGui::Text("Visible Models: %d", renderer->m_VisibleModels);
    }
    ImGui::End();
}


void StartingScene::BuildLightData() {
    Renderer* renderer = &Renderer::instance();
    Logger* logger = &Logger::instance();

    auto prev_view = registry.view<Rendering::LightData>();
    registry.destroy(prev_view.begin(), prev_view.end());
    logger->AddLog("[Scene] Rebuilt light registry, new size: %d", registry.size());

    std::mt19937 gen;
    std::uniform_real_distribution<> urd_red(0.5, 1.f);
    std::uniform_real_distribution<> urd_green(0.2, 0.8);
    std::uniform_real_distribution<> urd_blue(0.1, 0.5);

    std::uniform_real_distribution<> urd_pos_x(-spread, spread);
    std::uniform_real_distribution<> urd_pos_y(-0.8f, 4.f);
    std::uniform_real_distribution<> urd_pos_z(-spread, spread);

    const float constant = renderer->m_Settings.light_constant;
    const float linear = renderer->m_Settings.light_linear;
    const float quadratic = renderer->m_Settings.light_quadratic;
    const float intensity = renderer->m_Settings.intensity;

    std::vector<entt::entity> lights;
    lights.resize(LIGHT_COUNT_MAX);
    registry.create(lights.begin(), lights.end());
    std::cout << "registry size on creation" << registry.size() << std::endl;

    for (auto& light : lights) {
        auto& light_component = registry.emplace<Rendering::LightData>(light);

        float xPos = urd_pos_x(gen);
        float yPos = urd_pos_y(gen);
        float zPos = urd_pos_z(gen);
        light_component.Position = glm::vec4(xPos, yPos, zPos, 1.0);

        float rColor = urd_red(gen);
        float gColor = urd_green(gen);
        float bColor = urd_blue(gen);

        light_component.Color = glm::vec4(rColor, gColor, bColor, 1.0);
        light_component.Linear = linear;
        light_component.Quadratic = quadratic;
        const float maxBrightness = std::fmaxf(std::fmaxf(light_component.Color.r, light_component.Color.g), light_component.Color.b);
        float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - ((256.0f / 5.f) * maxBrightness)))) / (2.0f * quadratic);
        light_component.Radius = intensity * radius;
    }
}