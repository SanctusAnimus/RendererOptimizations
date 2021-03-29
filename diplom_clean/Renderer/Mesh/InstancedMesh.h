#pragma once
#ifndef INSTANCEDMESH_CLASS_DECL
#define INSTANCEDMESH_CLASS_DECL

#include <vector>

#include "../Texture.h"
#include "../Vertex.h"
#include "../Shader.h"

class InstancedMesh
{
public:
    // mesh data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture>      textures;

    InstancedMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void Bind(Shader* shader);
    void UpdateModels(glm::mat4* model_ptr, int count);
    void Unbind();
private:
    //  render data
    unsigned int VAO, VBO, EBO, m_ABO;

    void setupMesh();
};

#endif // !INSTANCEDMESH_CLASS_DECL


