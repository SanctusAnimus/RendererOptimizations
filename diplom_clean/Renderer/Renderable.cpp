#include "Renderable.h"
#include "Renderer.h"


void Renderable::SetNormalMap(std::string tex_name) {
    m_NormalMap = Renderer::instance().GetTexture(tex_name);
    std::cout << "set normal map to " << m_NormalMap << std::endl;
}

void Renderable::SetSpecularMap(std::string tex_name) {
    m_SpecularMap = Renderer::instance().GetTexture(tex_name);
    std::cout << "set specular map to " << m_SpecularMap << std::endl;
}

void Renderable::SetHeightMap(std::string tex_name) {
    m_HeightMap = Renderer::instance().GetTexture(tex_name);
    std::cout << "set height map to " << m_HeightMap << std::endl;
}