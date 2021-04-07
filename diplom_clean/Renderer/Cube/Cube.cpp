#include "Cube.h"
#include "../Renderer.h"


Cube::Cube(std::shared_ptr<Texture> texture, std::shared_ptr<Shader> shader) : Renderable(texture, shader) {

    static GLfloat vertices[] = {
    1.0,	1.0,	1.0,
    0.0f,	1.0,	1.0,
    1.0,	1.0,	0.0f,
    0.0f,	1.0,	0.0f,
    1.0,	0.0f,	1.0,
    0.0f,	0.0f,	1.0,
    0.0f,	0.0f,	0.0f,
    1.0,	0.0f,	0.0f
    };


    unsigned int indices[] = {
        3, 2, 6, 7, 4, 2, 0,
        3, 1, 6, 5, 4, 1, 0
    };


    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
}


void Cube::Render(std::shared_ptr<Camera::BaseCamera> camera, glm::mat4 projection) {

}

void Cube::SetNormalMap(std::string tex_name) {
    m_NormalMap = Renderer::instance().GetTexture(tex_name);
    std::cout << "set normal map to " << m_NormalMap << std::endl;
}

void Cube::SetSpecularMap(std::string tex_name) {
    m_SpecularMap = Renderer::instance().GetTexture(tex_name);
    std::cout << "set specular map to " << m_SpecularMap << std::endl;
}

void Cube::SetHeightMap(std::string tex_name) {
    m_HeightMap = Renderer::instance().GetTexture(tex_name);
    std::cout << "set height map to " << m_HeightMap << std::endl;
}