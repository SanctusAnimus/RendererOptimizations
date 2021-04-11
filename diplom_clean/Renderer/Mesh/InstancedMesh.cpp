#include "InstancedMesh.h"

InstancedMesh::InstancedMesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<Texture>> m_Textures, unsigned int ABO)
{
    // std::cout << "[Instanced Mesh] created" << std::endl;
    this->vertices = vertices;
    this->indices = indices;
    this->m_Textures = m_Textures;

    setupMesh(ABO);
}


InstancedMesh::~InstancedMesh() {
    // std::cout << "[Instanced Mesh] deleted" << std::endl;
    vertices.clear();
    indices.clear();
    m_Textures.clear();
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void InstancedMesh::setupMesh(unsigned int ABO)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);

    // glBindBuffer(GL_ARRAY_BUFFER, m_ABO);
    // glBufferData(GL_ARRAY_BUFFER, models.size() * 4 * sizeof(glm::vec4), &models[0], GL_DYNAMIC_DRAW);

    // vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

    std::size_t vec4Size = sizeof(glm::vec4);
    // position matrixes
    glBindBuffer(GL_ARRAY_BUFFER, ABO);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glVertexAttribDivisor(5, 1);

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glVertexAttribDivisor(6, 1);

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glVertexAttribDivisor(7, 1);

    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
    glVertexAttribDivisor(8, 1);

    glBindVertexArray(0);
}

void InstancedMesh::Bind(std::shared_ptr<Shader> shader) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    glBindVertexArray(VAO);
    for (unsigned int i = 0; i < m_Textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = m_Textures[i]->type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);
        else if (name == "texture_normal")
            number = std::to_string(normalNr++);

        shader->setInt((name + number).c_str(), i);
        glBindTexture(GL_TEXTURE_2D, m_Textures[i]->id);
    }
 
}

void InstancedMesh::Unbind() {
    glBindVertexArray(0);
    for (unsigned int i = 0; i < m_Textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}


void InstancedMesh::UpdateModels(glm::mat4* model_ptr, int count) {
    // glBindBuffer(GL_ARRAY_BUFFER, m_ABO);
    // glBufferData(GL_ARRAY_BUFFER, count * 4 * sizeof(glm::vec4), model_ptr, GL_STREAM_DRAW);
}