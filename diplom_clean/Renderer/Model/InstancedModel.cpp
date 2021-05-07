#include "InstancedModel.h"
#include "../Renderer.h"
#include <imgui/imgui.h>
#include <GLFW/glfw3.h>


bool InstancedModel::_enable_frustum = true;

bool IsVisible(glm::vec4 worldspace, float radius) {
    // return true;
    return abs(worldspace.x) < worldspace.w + radius &&
        abs(worldspace.y) < worldspace.w + radius &&
        radius < worldspace.z &&
        abs(worldspace.z) < worldspace.w + radius;
}

InstancedModel::InstancedModel(const char* path, std::shared_ptr<Shader> shader, bool compress) : m_Shader(shader), m_HasCompressedTextures(compress) {
    std::cout << "[Instanced Model] creation started!" << std::endl;
	loadModel(path);
    std::cout << "[Instanced Model] creation finished!" << std::endl;
    std::cout << meshes.size() << std::endl;
}

InstancedModel::~InstancedModel() {
    std::cout << "[Instanced Model] destructor called!" << std::endl;
    meshes.clear();
    instance_traits.clear();
    glDeleteBuffers(1, &m_ABO);
}

void InstancedModel::Render(std::shared_ptr<Camera::BaseCamera> camera, glm::mat4 projection)
{
    size_t positions_count = instance_traits.size();
    auto positions_ptr = &instance_traits[0];

    glm::mat4 view = camera->GetViewMatrix();

    this->m_Shader->use();
    this->m_Shader->setVec3("viewPos", camera->m_Position);
    this->m_Shader->setMat4("view", view);
    this->m_Shader->setMat4("projection", projection);
    this->m_Shader->setFloat("time", (float)glfwGetTime());

    Renderer* renderer = &Renderer::instance();

    float radius = renderer->m_Settings.models_sphere_radius;

    glm::mat4 mvp = projection * view * glm::mat4(1.f);

    if (m_RenderLimit >= (int)instance_traits.size()) m_RenderLimit = (int)instance_traits.size();

    std::vector<InstanceTraits>& ref = instance_traits;
    std::vector<InstanceTraits> visible_instances;
    size_t index = 0;
    for (const auto& trait : instance_traits) {
        if (index++ >= m_RenderLimit) break;
        glm::vec4 worldspace = mvp * glm::vec4(trait.transform[3]);
        if (!this->_enable_frustum || IsVisible(worldspace, radius)) {
            visible_instances.push_back(trait);
        }
    }

    size_t visible_count = visible_instances.size();
    renderer->m_VisibleModels = visible_count;

    glBindBuffer(GL_ARRAY_BUFFER, m_ABO);
    glBindBuffer(GL_ARRAY_BUFFER, m_ABO);
    glBufferData(GL_ARRAY_BUFFER, visible_count * sizeof(InstanceTraits), &visible_instances[0], GL_STREAM_DRAW);

    m_Shader->setInt("has_render_color", (int)m_HasRenderColor);

    for (auto& mesh : meshes) {
        // mesh.UpdateModels(&visible_positions[0], visible_count);
        mesh->Bind(m_Shader);
        glDrawElementsInstanced(
            GL_TRIANGLES, (GLsizei)mesh->indices.size(), GL_UNSIGNED_INT, 0, (GLsizei)visible_count
        );
        mesh->Unbind();
    }
}

int InstancedModel::Add(glm::mat4 position, glm::vec4 color)
{
    size_t index = instance_traits.size();
    instance_traits.push_back({ position, color });
    return static_cast<int>(index);
}

void InstancedModel::Update(int index, glm::mat4 new_model) {
    if (index < instance_traits.size()) {
        instance_traits[index].transform = new_model;
    }
}

void InstancedModel::loadModel(std::string path)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    size_t last_of = path.find_last_of('/');
    m_ModelName = path.substr(last_of + 1);
    m_Directory = path.substr(0, last_of);

    glGenBuffers(1, &m_ABO);

    processNode(scene->mRootNode, scene);
}

void InstancedModel::processNode(aiNode* node, const aiScene* scene)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

void InstancedModel::UI_Description() {
    auto instance_traits_size = static_cast<int>(instance_traits.size());
    ImGui::PushID(m_ModelName.c_str());
    ImGui::TextUnformatted("Type: Model");
    ImGui::Text("Name: %s", m_ModelName.c_str());
    ImGui::Checkbox("Colored", &m_HasRenderColor);
    ImGui::SliderInt("Render Limit", &m_RenderLimit, 0, instance_traits_size);
    ImGui::Text("Controlled entities: %d", instance_traits_size);
    ImGui::PopID();
}

std::shared_ptr<InstancedMesh> InstancedModel::processMesh(aiMesh* mesh, const aiScene* scene)
{
    // data to fill
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<std::shared_ptr<Texture>> m_Textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.m_Position = vector;
        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        // texture coordinates
        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    using texture_vector = std::vector<std::shared_ptr<Texture>>;
    texture_vector diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", m_HasCompressedTextures);
    m_Textures.insert(m_Textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    texture_vector specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", m_HasCompressedTextures);
    m_Textures.insert(m_Textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    texture_vector normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", false);
    m_Textures.insert(m_Textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. emission maps
    texture_vector emission_maps = loadMaterialTextures(material, aiTextureType_EMISSION_COLOR, "texture_emission", false);
    m_Textures.insert(m_Textures.end(), emission_maps.begin(), emission_maps.end());
    // 4. emission maps
    texture_vector emissive_maps = loadMaterialTextures(material, aiTextureType_EMISSIVE, "texture_emissive", false);
    m_Textures.insert(m_Textures.end(), emissive_maps.begin(), emissive_maps.end());
    // 5. height maps
    texture_vector heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height", m_HasCompressedTextures);
    m_Textures.insert(m_Textures.end(), heightMaps.begin(), heightMaps.end());

    // return a mesh object created from the extracted mesh data
    return std::shared_ptr<InstancedMesh>(new InstancedMesh(vertices, indices, m_Textures, m_ABO));
}


std::vector<std::shared_ptr<Texture>> InstancedModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const bool compress)
{
    Renderer* renderer = &Renderer::instance();
    std::vector<std::shared_ptr<Texture>> m_Textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string path = str.C_Str();
        
        std::shared_ptr<Texture> texture = renderer->GetTexture(path);
        if (texture == nullptr) {
            texture = renderer->NewTexture(
                (m_Directory + '/' + path).c_str(), path, typeName, compress
            );
            if (texture == nullptr) std::cout << "NEW TEXTURE IS NULLPTR!" << std::endl;
            std::cout << "Loaded texture " << typeName << " with path " << texture->path << ", compress state: " << compress << std::endl;
        }
        m_Textures.push_back(texture);
    }
    return m_Textures;
}