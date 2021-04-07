#pragma once
#ifndef MESH_CLASS_DECL
#define MESH_CLASS_DECL
#include <vector>

#include "../Texture.h"
#include "../Vertex.h"
#include "../Shader.h"

class Mesh {
public:
    // mesh data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<std::shared_ptr<Texture>>      m_Textures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<Texture>> m_Textures);
    ~Mesh();
    void Render(std::shared_ptr<Shader> shader);
private:
    //  render data
    unsigned int VAO, VBO, EBO;

    void setupMesh();
};


#endif // !MESH_CLASS_DECL