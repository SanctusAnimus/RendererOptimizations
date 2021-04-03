#pragma once
#ifndef VERTEX_STRUCT_DECL
#define VERTEX_STRUCT_DECL

#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 m_Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct BasicVertex {
    glm::vec3 m_Position;
};

struct TexturedVertex {
    glm::vec3 m_Position;
    glm::vec2 m_TexCoords;
};

struct ColoredVertex {
    glm::vec3 m_Position;
    glm::vec3 m_Color;
};

#endif // !VERTEX_STRUCT_DECL