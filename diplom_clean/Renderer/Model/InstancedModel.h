#pragma once
#ifndef INSTANCEDMODEL_CLASS_DECL
#define INSTANCEDMODEL_CLASS_DECL

#include "../Renderable.h"
#include "../Mesh/InstancedMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


class InstancedModel
{
public:
    InstancedModel(const char* path, std::shared_ptr<Shader> shader, bool compress);
    ~InstancedModel();
    int Add(glm::mat4 position, glm::vec4 color = glm::vec4(0.f, 0.f, 0.f, 1.f));
    void Set() {};
    void Update(int index, glm::mat4 new_model);
    void SetInstanceColor(int index, glm::vec4& new_color);
    void Render(std::shared_ptr<Camera::BaseCamera> camera, glm::mat4 projection);
    void UI_Description();

    void SetRenderLimit(unsigned int new_limit) { m_RenderLimit = new_limit; }

    void SetHasRenderColor(const bool& flag) {
        m_HasRenderColor = flag;
    }
    static bool _enable_frustum;
private:

    // model data
    std::shared_ptr<Shader> m_Shader;
    std::vector<std::shared_ptr<InstancedMesh>> meshes;
    std::vector<InstanceTraits> instance_traits;

    std::string m_ModelName;
    std::string m_Directory;

    unsigned int m_ABO;
    int m_RenderLimit = 1024;
    bool m_HasRenderColor = false;
    bool m_HasCompressedTextures = false;

    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    std::shared_ptr<InstancedMesh> processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const bool compress = true);
};

#endif // !INSTANCEDMODEL_CLASS_DECL



