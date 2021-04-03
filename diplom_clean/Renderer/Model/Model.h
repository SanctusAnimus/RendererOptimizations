#pragma once
#ifndef MODEL_CLASS_DECL
#define MODEL_CLASS_DECL

#include "../Shader.h"
#include "../Mesh/Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


class Model
{
public:
    std::vector<Texture> textures_loaded;
    Model(const char* path)
    {
        loadModel(path);
    }
    void Render(std::shared_ptr<Shader> shader);
private:
    // model data
    std::vector<Mesh> meshes;
    std::string directory;

    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

#endif // !MODEL_CLASS_DECL