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
    InstancedModel(const char* path, Shader* shader);
    void Add(glm::mat4 position);
    void Render(std::shared_ptr<Camera::BaseCamera> camera, glm::mat4 projection);
    void UI_Description();
private:
    // model data
    Shader* m_Shader;
    std::vector<InstancedMesh> meshes;
    std::vector<glm::mat4> positions;
    std::string directory;

    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    InstancedMesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const bool gamma = false);
};

#endif // !INSTANCEDMODEL_CLASS_DECL



