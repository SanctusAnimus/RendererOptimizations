#include "InstancedModel.h"
#include "../Renderer.h"
#include <imgui/imgui.h>
#include <GLFW/glfw3.h>


bool IsVisible(glm::vec4 worldspace, float radius) {
    return abs(worldspace.x) < worldspace.w + radius &&
        abs(worldspace.y) < worldspace.w + radius &&
        radius < worldspace.z &&
        abs(worldspace.z) < worldspace.w + radius;
}

InstancedModel::InstancedModel(const char* path, Shader* shader) : m_Shader(shader) {
	loadModel(path);
}

InstancedModel::~InstancedModel() {
    std::cout << "Instanced Model destructor called!" << std::endl;
    meshes.clear();
    positions.clear();
    glDeleteBuffers(1, &m_ABO);
}

void InstancedModel::Render(std::shared_ptr<Camera::BaseCamera> camera, glm::mat4 projection)
{
    int count = positions.size();
    int positions_count = positions.size();
    auto positions_ptr = &positions[0];

    glm::mat4 view = camera->GetViewMatrix();

    this->m_Shader->use();
    this->m_Shader->setVec3("viewPos", camera->m_Position);
    this->m_Shader->setMat4("view", view);
    this->m_Shader->setMat4("projection", projection);
    this->m_Shader->setFloat("time", glfwGetTime());

    Renderer* renderer = &Renderer::instance();

    float radius = renderer->m_Settings.models_sphere_radius;

    glm::mat4 mvp = projection * view * glm::mat4(1.f);

    std::vector<glm::mat4> visible_positions;

    for (const auto& mat : positions) {
        glm::vec4 worldspace = mvp * glm::vec4(mat[3]);
        if (IsVisible(worldspace, radius)) {
            visible_positions.push_back(mat);
        }
    }
    int visible_count = visible_positions.size();
    renderer->m_VisibleModels = visible_count;

    glBindBuffer(GL_ARRAY_BUFFER, m_ABO);
    glBindBuffer(GL_ARRAY_BUFFER, m_ABO);
    glBufferData(GL_ARRAY_BUFFER, visible_count * 4 * sizeof(glm::vec4), &visible_positions[0], GL_STREAM_DRAW);

    for (auto& mesh : meshes) {
        // mesh.UpdateModels(&visible_positions[0], visible_count);
        mesh.Bind(m_Shader);
        glDrawElementsInstanced(
            GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0, visible_count
        );
        mesh.Unbind();
    }
}

void InstancedModel::Add(glm::mat4 position)
{
    positions.push_back(position);
    auto ptr = &positions[0];
    int size = positions.size();
    for (auto& mesh : meshes) {
        mesh.UpdateModels(&positions[0], size);
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
    directory = path.substr(0, path.find_last_of('/'));

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
    ImGui::TextUnformatted("Type: Model");
    ImGui::Text("Controlled entities: %d", positions.size());
}

InstancedMesh InstancedModel::processMesh(aiMesh* mesh, const aiScene* scene)
{
    // data to fill
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> m_Textures;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
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
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
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
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 1. diffuse maps
    std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    m_Textures.insert(m_Textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    m_Textures.insert(m_Textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    m_Textures.insert(m_Textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    m_Textures.insert(m_Textures.end(), heightMaps.begin(), heightMaps.end());

    // return a mesh object created from the extracted mesh data
    return InstancedMesh(vertices, indices, m_Textures, m_ABO);
}


std::vector<Texture> InstancedModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const bool gamma)
{
    Renderer* renderer = &Renderer::instance();
    std::vector<Texture> m_Textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string path = str.C_Str();
        
        Texture* texture = renderer->GetTexture(path);
        if (texture == nullptr) {
            texture = renderer->NewTexture(
                (directory + '/' + path).c_str(), path, typeName, gamma
            );
            std::cout << "Loaded texture " << typeName << " with path " << texture->path << std::endl;
        }
        m_Textures.push_back(*texture);
    }
    return m_Textures;
}