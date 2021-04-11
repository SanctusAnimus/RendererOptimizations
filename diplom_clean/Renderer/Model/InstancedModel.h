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
    InstancedModel(const char* path, std::shared_ptr<Shader> shader);
    ~InstancedModel();
    int Add(glm::mat4 position);
    void Set() {};
    void Update(int index, glm::mat4 new_model);
    void Render(std::shared_ptr<Camera::BaseCamera> camera, glm::mat4 projection);
    void UI_Description();
private:
    // model data
    std::shared_ptr<Shader> m_Shader;
    std::vector<std::shared_ptr<InstancedMesh>> meshes;
    std::vector<glm::mat4> positions;
    std::string directory;

    unsigned int m_ABO;

    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    std::shared_ptr<InstancedMesh> processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const bool compress = true);
};

#endif // !INSTANCEDMODEL_CLASS_DECL



