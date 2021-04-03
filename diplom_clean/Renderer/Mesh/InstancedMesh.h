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
    std::vector<Texture>      m_Textures;

    InstancedMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> m_Textures, unsigned int ABO);
    ~InstancedMesh();

    void Bind(std::shared_ptr<Shader> shader);
    void UpdateModels(glm::mat4* model_ptr, int count);
    void Unbind();
private:
    //  render data
    unsigned int VAO, VBO, EBO;

    void setupMesh(unsigned int ABO);
};

#endif // !INSTANCEDMESH_CLASS_DECL


