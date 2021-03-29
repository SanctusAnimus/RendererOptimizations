#include "StartingScene.h"
#include <glad/glad.h>
#include <iostream>
#include <imgui/imgui.h>
#include <random>

#include "../Renderer/Renderer.h"


void StartingScene::Setup() {
    std::cout << "Scene setup called" << std::endl;
    Renderer* renderer = &Renderer::instance();

    auto arcball_camera = renderer->NewCamera({ 0.0f, 0.0f, 3.0f }, "arcball_camera", Camera::Camera_Type::ARCBALL);
    auto fly_camera = renderer->NewCamera({ 0.0f, 0.0f, 3.0f }, "fly_camera", Camera::Camera_Type::FLYCAM);
    renderer->SetActiveCamera("arcball_camera");

    Texture* quad_tex = renderer->NewTexture(
        "Resource/textures/brickwall.jpg", "brickwall", "texture_diffuse"
        // "Resource/textures/plate/Metal_Plate_Sci-Fi_002_basecolor.jpg", "brickwall", "texture_diffuse"
    );
    Texture* quad_normal = renderer->NewTexture(
        "Resource/textures/brickwall_normal.jpg", "brickwall_normal", "texture_normal"
        // "Resource/textures/plate/Metal_Plate_Sci-Fi_002_normal.jpg", "brickwall_normal", "texture_normal"
    );

    Texture* quad_specular = renderer->NewTexture(
        "Resource/textures/brickwall_specular.jpg", "brickwall_specular", "texture_specular"
    );

    Texture* quad_height = renderer->NewTexture(
        // "Resource/textures/bricks2_disp.jpg", "brickwall_displacement", "texture_displacement"
        "Resource/textures/plate/Metal_Plate_Sci-Fi_002_roughness.jpg", "plate_specular", "texture_height"
    );

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


    auto instanced_brick_wall = renderer->NewInstancedQuad("brickwall", "instanced_g_pass", "brickwall_instanced");
    instanced_brick_wall->SetNormalMap("brickwall_normal");
    instanced_brick_wall->SetSpecularMap("brickwall_specular");

    auto quad = renderer->NewQuad("brickwall", "g_pass");
    quad->SetNormalMap("brickwall_normal");
    // quad->SetSpecularMap("brickwall_specular");
    quad->SetRotation(-90, { 1.f, 0.f, 0.f });
    quad->SetScale({ 20.f, 20.f, 20.0 });
    quad->SetTransform({ 0.f, -1.f, 0.f });

    auto instanced_backpack = renderer->NewInstancedModel("Models/backpack/backpack.obj", "instanced_g_pass", "instanced_backpack");

    int count = 2;
    for (int i = -count; i < count; i++) {
        for (int j = -count; j < count; j++) {
            glm::vec3 translate(i * 3.f, 1.f, j * 3.f);
            glm::mat4 model(1.f);
            model = glm::mat4(1.0f);
            model = glm::translate(model, translate);
            model = glm::scale(model, glm::vec3(0.5f));

            instanced_backpack->Add(model);
        }
    }


    // configure g-buffer framebuffer
    // ------------------------------
    
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    
    int SCR_WIDTH = Rendering::SCREEN_WIDTH;
    int SCR_HEIGHT = Rendering::SCREEN_HEIGHT;

    

    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Rendering::SCREEN_WIDTH, Rendering::SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
   
    std::mt19937 gen;
    std::uniform_real_distribution<> urd(0.7, 1);

    for (unsigned int i = 0; i < LIGHT_COUNT_MAX; i++)
    {
        Rendering::LightData m_light_data;
        // calculate slightly random offsets
        float xPos = ((rand() % 100) / 100.0) * spread - spread / 2.0;
        float yPos = ((rand() % 100) / 100.0) * 5.0 - 1.0;
        float zPos = ((rand() % 100) / 100.0) * spread - spread / 2.0;
        m_light_data.Position = glm::vec4(xPos, yPos, zPos, 1.0);
        // lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
        // also calculate random color
        float rColor = urd(gen);
        float gColor = urd(gen);
        float bColor = urd(gen);
        // lightColors.push_back(glm::vec3(rColor, gColor, bColor));
        m_light_data.Color = glm::vec4(rColor, gColor, bColor, 1.0);

        const float constant = renderer->settings.light_constant;
        const float linear = renderer->settings.light_linear;
        const float quadratic = renderer->settings.light_quadratic;
        const float intensity = renderer->settings.intensity;
        m_light_data.Linear = linear;
        m_light_data.Quadratic = quadratic;
        const float maxBrightness = std::fmaxf(std::fmaxf(m_light_data.Color.r, m_light_data.Color.g), m_light_data.Color.b);
        float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - ((256.0f / 5.f) * maxBrightness)))) / (2.0f * quadratic);
        m_light_data.Radius = intensity * radius;
        light_data.push_back({ m_light_data });
    }
    lighting_pass_shader->use();

    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Rendering::LightData) * light_data.size(), &light_data[0], GL_STREAM_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    lighting_pass_shader->setInt("gPosition", 0);
    lighting_pass_shader->setInt("gNormal", 1);
    lighting_pass_shader->setInt("gAlbedoSpec", 2);

    std::cout << "...Done" << std::endl;



    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0
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

    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    blur_shader->use();
    blur_shader->setInt("image", 0);
    final_shader->use();
    final_shader->setInt("scene", 0);
    final_shader->setInt("bloomBlur", 1);
}

void StartingScene::Render() {
    Renderer* renderer = &Renderer::instance();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. geometry pass: render scene's geometry/color data into gbuffer
    // -----------------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(
        glm::radians(renderer->current_camera->m_Zoom),
        (float)Rendering::SCREEN_WIDTH / (float)Rendering::SCREEN_HEIGHT,
        0.1f, 100.0f
    );
    glm::mat4 view = renderer->current_camera->GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    glm::mat4 mvp = projection * view * model;

    std::vector<Rendering::LightData> temporary_light_data;
    this->RegenerateLights();
    for (auto& light : light_data) {
        if (light.IsVisibleFrom(mvp)) {
            temporary_light_data.push_back(light.GetData());
        }
    }

    renderer->visible_lights = temporary_light_data.size();

    geometry_pass_shader->use();
    geometry_pass_shader->setMat4("projection", projection);
    geometry_pass_shader->setMat4("view", view);
    glPolygonMode(GL_FRONT_AND_BACK, renderer->settings.wireframe ? GL_LINE : GL_FILL);
    /*
    for (unsigned int i = 0; i < objectPositions.size(); i++)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, objectPositions[i]);
        model = glm::scale(model, glm::vec3(0.5f));
        geometry_pass_shader->setMat4("model", model);
        model = glm::inverse(model);
        model = glm::transpose(model);
        geometry_pass_shader->setMat4("normal_model", model);
        backpack.Render((*geometry_pass_shader));
    }
    */
    renderer->Render();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
    // -----------------------------------------------------------------------------------------------------------------------
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    lighting_pass_shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    // send light relevant uniforms
    // MOVED OUT FROM LOOP
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Rendering::LightData) * temporary_light_data.size(), &temporary_light_data[0], GL_STREAM_DRAW);

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    lighting_pass_shader->setInt("light_count", temporary_light_data.size());
    lighting_pass_shader->setFloat("intensity", renderer->settings.intensity);
    lighting_pass_shader->setVec3("viewPos", renderer->current_camera->m_Position);
    // finally render quad
    renderer->SimpleQuad();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
    // ----------------------------------------------------------------------------------
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO); // write to default framebuffer
    glBlitFramebuffer(0, 0, 
        Rendering::SCREEN_WIDTH, Rendering::SCREEN_HEIGHT, 0, 0, 
        Rendering::SCREEN_WIDTH, Rendering::SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    // 3. render lights on top of scene
    // --------------------------------
    light_box_shader->use();
    light_box_shader->setMat4("projection", projection);
    light_box_shader->setMat4("view", view);

    for (unsigned int i = 0; i < temporary_light_data.size(); i++)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(temporary_light_data[i].Position));
        model = glm::scale(model, glm::vec3(0.055f));
        light_box_shader->setMat4("model", model);
        light_box_shader->setVec3("lightColor", glm::vec3(temporary_light_data[i].Color));
        cube.Render((*light_box_shader));
        // renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    bool horizontal = true, first_iteration = true;
    unsigned int amount = 10;
    blur_shader->use();
    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        blur_shader->setInt("horizontal", horizontal);
        // glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
        renderer->SimpleQuad();
        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    final_shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
    final_shader->setInt("bloom", true);
    final_shader->setFloat("exposure", 1.f);
    renderer->SimpleQuad();

    renderer->GatherImGui();

    if (ImGui::Begin("Buffer View")) {
        const auto size = ImVec2(Rendering::SCREEN_WIDTH / 10.0, Rendering::SCREEN_HEIGHT / 10.0);
        // ImGui::Image((ImTextureID)gPosition, size, ImVec2(-1, 1), ImVec2(0, 0));
        // ImGui::Image((ImTextureID)gNormal, size, ImVec2(-1, 1), ImVec2(0, 0));
        ImGui::Image((ImTextureID)gAlbedoSpec, size, ImVec2(-1, 1), ImVec2(0, 0));

        ImGui::Image((ImTextureID)colorBuffers[0], size, ImVec2(1, 1), ImVec2(0, 0));
        ImGui::Image((ImTextureID)colorBuffers[1], size, ImVec2(1, 1), ImVec2(0, 0));

        ImGui::Image((ImTextureID)pingpongColorbuffers[0], size, ImVec2(1, 1), ImVec2(0, 0));
        ImGui::End();
    }
    else {
        ImGui::End();
    }
}

void StartingScene::RegenerateLights() {
    Renderer* renderer = &Renderer::instance();
    const float constant = renderer->settings.light_constant; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
    const float linear = renderer->settings.light_linear;
    const float quadratic = renderer->settings.light_quadratic;
    const float intensity = renderer->settings.intensity;


    for (auto& light : light_data) {

        light.data.Linear = linear;
        light.data.Quadratic = quadratic;
        const float maxBrightness = std::fmaxf(std::fmaxf(light.data.Color.r, light.data.Color.g), light.data.Color.b);
        float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - ((256.0f / 5.f) * maxBrightness)))) / (2.0f * quadratic);
        light.data.Radius = radius * intensity;
    }

}