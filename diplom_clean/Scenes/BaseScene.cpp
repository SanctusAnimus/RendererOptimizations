#include "BaseScene.h"
#include "../Renderer/Renderer.h"
#include "../ImGui/ImGuiLogger.h"
#include "../Components/Transform.h"
#include "../Components/Light.h"
#include "../Components/Render.h"

#include <entt/entt.hpp>
#include <random>

bool IsVisibleFrom(glm::vec4 position, glm::mat4& mvp, float radius) {
    glm::vec4 worldspace = mvp * position;
    return abs(worldspace.x) < worldspace.w + radius &&
        abs(worldspace.y) < worldspace.w + radius &&
        0 < worldspace.z &&
        abs(worldspace.z) < worldspace.w + radius;
}

std::vector<float> GetSkyboxVerts() {
    std::vector<float> arr = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    return arr;
}

void BaseScene::AddInstancedModel(entt::registry& registry, entt::entity entity) {
    Renderer* renderer = &Renderer::instance();

    auto& transform = registry.get<TransformComponent>(entity);
    auto& render = registry.get<InstancedModelComponent>(entity);
    auto light_component = registry.try_get<Rendering::LightData>(entity);

    auto instance_ctrl = renderer->GetInstancedModel(render.m_ModelName);
    if (instance_ctrl == nullptr) {
        instance_ctrl = renderer->NewInstancedModel(render.m_ModelName, render.m_ShaderName, render.m_ModelName);
    }

    if (light_component != nullptr) {
        render.m_InstanceIdx = instance_ctrl->Add(transform.GetModelMatrix(), light_component->Color);
        render.SetRenderColor(light_component->Color);
        instance_ctrl->SetHasRenderColor(true);
    }
    else {
        render.m_InstanceIdx = instance_ctrl->Add(transform.GetModelMatrix());
    }
}

void BaseScene::UpdateInstancedModel(entt::registry& registry, entt::entity entity) {
    Renderer* renderer = &Renderer::instance();

    auto& transform = registry.get<TransformComponent>(entity);
    auto render = registry.try_get<InstancedModelComponent>(entity);

    if (render != nullptr && render->m_InstanceIdx != -1) {
        auto instance_ctrl = renderer->GetInstancedModel(render->m_ModelName);
        if (instance_ctrl != nullptr) [[likely]] {
            instance_ctrl->Update(render->m_InstanceIdx, transform.GetModelMatrix());
        }
    }
}

void BaseScene::RegenerateLights(std::vector<Rendering::LightData>& visible_lights) {
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


void BaseScene::BuildLightData() {
    Renderer* renderer = &Renderer::instance();
    Logger* logger = &Logger::instance();

    auto prev_view = m_Registry.view<Rendering::LightData>();
    m_Registry.destroy(prev_view.begin(), prev_view.end());

    std::mt19937 gen;
    std::uniform_real_distribution<float> urd_red(0.5f, 1.f);
    std::uniform_real_distribution<float> urd_green(0.2f, 0.8f);
    std::uniform_real_distribution<float> urd_blue(0.1f, 0.5f);

    std::uniform_real_distribution<float> urd_pos_x(-spread, spread);
    std::uniform_real_distribution<float> urd_pos_y(-0.8f, 4.f);
    std::uniform_real_distribution<float> urd_pos_z(-spread, spread);

    const float constant = renderer->m_Settings.light_constant;
    const float linear = renderer->m_Settings.light_linear;
    const float quadratic = renderer->m_Settings.light_quadratic;
    const float intensity = renderer->m_Settings.intensity;

    std::vector<entt::entity> lights;
    lights.resize(LIGHT_COUNT_MAX);
    m_Registry.create(lights.begin(), lights.end());

    for (auto& light : lights) {
        auto& light_component = m_Registry.emplace<Rendering::LightData>(light);

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

        auto& tranform_component = m_Registry.emplace<TransformComponent>(light, glm::vec3{ light_component.Position }, glm::vec3{ 0.055f , 0.055f , 0.055f });
        auto& render_component = m_Registry.emplace<InstancedModelComponent>(light, "Models/Crate/Crate1.obj", "instanced_g_pass");
    }
}