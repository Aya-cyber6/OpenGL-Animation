#include "..//headers/SkinnedMesh.h"
#include <iostream>


void SkinnedMesh::Clear() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(NUM_BUFFERS, m_Buffers);

}

bool SkinnedMesh::LoadMesh(const std::string& filename) {

    Clear();

    // Create the VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // Create the buffers for the vertices attributes
    glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

    bool Ret = false;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);


    if (scene) {
        Ret = InitFromScene(scene, filename);
    }
    else {
        std::cerr << "Error parsing: " << filename << " - " << importer.GetErrorString() << std::endl;
        return false;
    }
    // Make sure the VAO is not changed from the outside
    glBindVertexArray(0);

    return Ret;
}

bool SkinnedMesh::InitFromScene(const aiScene* scene, const std::string& filename) {

    std::cout << "check POINT00"<< std::endl;

    m_Meshes.resize(scene->mNumMeshes);
    m_Materials.resize(scene->mNumMaterials);

    unsigned int numVertices = 0, numIndices = 0;

    CountVerticesAndIndices(scene, numVertices, numIndices);
    ReserveSpace(numVertices, numIndices);
    std::cout << "check POINT11" << std::endl;
    InitAllMeshes(scene);
    std::cout << "check POINT22" << std::endl;

    if (!InitMaterials(scene, filename)) {
        std::cout << "check POINT33" << std::endl;
        return false;

    }
    PopulateBuffers();
    std::cout << "check POINT44" << std::endl;

    return true;
}

void SkinnedMesh::CountVerticesAndIndices(const aiScene* scene, unsigned int& NumVertices, unsigned int& NumIndices) {
   
    for (unsigned int i = 0; i < m_Meshes.size(); ++i) {
        m_Meshes[i].MaterialIndex = scene->mMeshes[i]->mMaterialIndex;
        m_Meshes[i].NumIndices = scene->mMeshes[i]->mNumFaces * 3;
        m_Meshes[i].BaseVertex = NumVertices;
        m_Meshes[i].BaseIndex = NumIndices;

        NumVertices += scene->mMeshes[i]->mNumVertices;
        NumIndices += m_Meshes[i].NumIndices;
    }
}

void SkinnedMesh::ReserveSpace(unsigned int totalVertices, unsigned int totalIndices) {
    m_Positions.reserve(totalVertices);
    m_Normals.reserve(totalVertices);
    m_TexCoords.reserve(totalVertices);
    m_Bones.reserve(totalVertices);
    m_Indices.reserve(totalIndices);
}


void SkinnedMesh::InitAllMeshes(const aiScene* scene) {

    for (unsigned int i = 0; i < m_Meshes.size(); ++i) {
        const aiMesh* mesh = scene->mMeshes[i];
        InitSingleMesh(i, mesh);
    }
}

void SkinnedMesh::InitSingleMesh(unsigned int meshIndex, const aiMesh* mesh) {
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {

        const aiVector3D& pos = mesh->mVertices[i];
        m_Positions.emplace_back(pos.x, pos.y, pos.z);

        if (mesh->mNormals) {
            const aiVector3D& normal = mesh->mNormals[i];
            m_Normals.emplace_back(normal.x, normal.y, normal.z);
        }
        else {
            aiVector3D normal(0.0f, 1.0f, 0.0f);
            m_Normals.emplace_back(normal.x, normal.y, normal.z);
        }

        if (mesh->HasTextureCoords(0)) {
            const aiVector3D& tex = mesh->mTextureCoords[0][i];
            m_TexCoords.emplace_back(tex.x, tex.y);
        }
        else {
            m_TexCoords.emplace_back(0.0f, 0.0f);
        }
    }

    LoadMeshBones(meshIndex, mesh);

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        assert(face.mNumIndices == 3);
        m_Indices.push_back(face.mIndices[0]);
        m_Indices.push_back(face.mIndices[1]);
        m_Indices.push_back(face.mIndices[2]);
    }
}

void SkinnedMesh::LoadMeshBones(unsigned int meshIndex, const aiMesh* mesh) {
    for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
        LoadSingleBone(meshIndex, mesh->mBones[i]);
    }
}

void SkinnedMesh::LoadSingleBone(unsigned int meshIndex, const aiBone* bone) {
    int boneId = GetBoneId(bone);
    for (unsigned int i = 0; i < bone->mNumWeights; ++i) {
        const aiVertexWeight& vw = bone->mWeights[i];
        unsigned int GlobalVertexID = m_Meshes[meshIndex].BaseVertex + bone->mWeights[i].mVertexId;
        m_Bones[GlobalVertexID].AddBoneData(boneId, vw.mWeight);
    }
}

int SkinnedMesh::GetBoneId(const aiBone* bone) {


    std::string name(bone->mName.C_Str());
    auto it = m_BoneNameToIndexMap.find(name);
    if (it == m_BoneNameToIndexMap.end()) {
        int newId = static_cast<int>(m_BoneNameToIndexMap.size());
        m_BoneNameToIndexMap[name] = newId;
        return newId;
    }
    return it->second;
}


bool SkinnedMesh::InitMaterials(const aiScene* pScene, const std::string& Filename)
{

    std::string Dir = Filename.substr(0, Filename.find_last_of("/\\") == std::string::npos ? 0 : Filename.find_last_of("/\\"));
    if (Dir.empty()) Dir = ".";

    bool Ret = true;

    printf("Num materials: %d\n", pScene->mNumMaterials);

    // Initialize the materials
    for (unsigned int i = 0; i < pScene->mNumMaterials; i++) {
        const aiMaterial* pMaterial = pScene->mMaterials[i];

        LoadTextures(Dir, pMaterial, i);

        LoadColors(pMaterial, i);
    }

    return Ret;
}


void SkinnedMesh::LoadTextures(const std::string& Dir, const aiMaterial* pMaterial, int index)
{
    LoadDiffuseTexture(Dir, pMaterial, index);
    LoadSpecularTexture(Dir, pMaterial, index);
}


void SkinnedMesh::LoadDiffuseTexture(const std::string& Dir, const aiMaterial* pMaterial, int index)
{
    m_Materials[index].diffuseMap = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString Path;

        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            std::string p(Path.data);

            if (p.substr(0, 2) == ".\\") {
                p = p.substr(2, p.size() - 2);
            }

            std::string FullPath = Dir + "/" + p;

            m_Materials[index].diffuseMap = new Texture(GL_TEXTURE_2D, FullPath.c_str());

            if (!m_Materials[index].diffuseMap->Load()) {
                printf("Error loading diffuse texture '%s'\n", FullPath.c_str());
                exit(0);
            }
            else {
                printf("Loaded diffuse texture '%s'\n", FullPath.c_str());
            }
        }
    }
}


void SkinnedMesh::LoadSpecularTexture(const std::string& Dir, const aiMaterial* pMaterial, int index)
{
    m_Materials[index].pSpecularExponent = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0) {
        aiString Path;

        if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            std::string p(Path.data);

            if (p == "C:\\\\") {
                p = "";
            }
            else if (p.substr(0, 2) == ".\\") {
                p = p.substr(2, p.size() - 2);
            }

            std::string FullPath = Dir + "/" + p;

            m_Materials[index].pSpecularExponent = new Texture(GL_TEXTURE_2D, FullPath.c_str());

            if (!m_Materials[index].pSpecularExponent->Load()) {
                printf("Error loading specular texture '%s'\n", FullPath.c_str());
                exit(0);
            }
            else {
                printf("Loaded specular texture '%s'\n", FullPath.c_str());
            }
        }
    }
}

void SkinnedMesh::LoadColors(const aiMaterial* pMaterial, int index)
{
    aiColor4D AmbientColor(0.0f, 0.0f, 0.0f, 0.0f);
    glm::vec4 AllOnes(1.0f, 1.0f, 1.0f, 1.0);

    int ShadingModel = 0;
    if (pMaterial->Get(AI_MATKEY_SHADING_MODEL, ShadingModel) == AI_SUCCESS) {
        printf("Shading model %d\n", ShadingModel);
    }

    if (pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, AmbientColor) == AI_SUCCESS) {
        printf("Loaded ambient color [%f %f %f]\n", AmbientColor.r, AmbientColor.g, AmbientColor.b);
        m_Materials[index].AmbientColor.r = AmbientColor.r;
        m_Materials[index].AmbientColor.g = AmbientColor.g;
        m_Materials[index].AmbientColor.b = AmbientColor.b;
    }
    else {
        m_Materials[index].AmbientColor = AllOnes;
    }

    aiColor3D DiffuseColor(0.0f, 0.0f, 0.0f);

    if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS) {
        printf("Loaded diffuse color [%f %f %f]\n", DiffuseColor.r, DiffuseColor.g, DiffuseColor.b);
        m_Materials[index].DiffuseColor.r = DiffuseColor.r;
        m_Materials[index].DiffuseColor.g = DiffuseColor.g;
        m_Materials[index].DiffuseColor.b = DiffuseColor.b;
    }

    aiColor3D SpecularColor(0.0f, 0.0f, 0.0f);

    if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, SpecularColor) == AI_SUCCESS) {
        printf("Loaded specular color [%f %f %f]\n", SpecularColor.r, SpecularColor.g, SpecularColor.b);
        m_Materials[index].SpecularColor.r = SpecularColor.r;
        m_Materials[index].SpecularColor.g = SpecularColor.g;
        m_Materials[index].SpecularColor.b = SpecularColor.b;
    }
}


void SkinnedMesh::PopulateBuffers() {
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, m_Positions.size() * sizeof(glm::vec3), m_Positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);


    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, m_TexCoords.size() * sizeof(glm::vec2), m_TexCoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);


    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, m_Normals.size() * sizeof(glm::vec3), m_Normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);


    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BONE_VB]);
    glBufferData(GL_ARRAY_BUFFER, m_Bones.size() * sizeof(VertexBoneData), m_Bones.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_UNSIGNED_INT, sizeof(VertexBoneData), (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (void*)(sizeof(unsigned int) * 4));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), m_Indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void SkinnedMesh::Render() {
    glBindVertexArray(m_VAO);

    for (unsigned int i = 0; i < m_Meshes.size(); i++) {
        unsigned int MaterialIndex = m_Meshes[i].MaterialIndex;

        assert(MaterialIndex < m_Materials.size());

        if (m_Materials[MaterialIndex].diffuseMap) {
            m_Materials[MaterialIndex].diffuseMap->Bind(GL_TEXTURE0);
        }

        if (m_Materials[MaterialIndex].pSpecularExponent) {
            m_Materials[MaterialIndex].pSpecularExponent->Bind(GL_TEXTURE8);
        }

        glDrawElementsBaseVertex(GL_TRIANGLES,
            m_Meshes[i].NumIndices,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned int) * m_Meshes[i].BaseIndex),
            m_Meshes[i].BaseVertex);
    }

    glBindVertexArray(0);
}


const Material& SkinnedMesh::GetMaterial()
{
    for (unsigned int i = 0; i < m_Materials.size(); i++) {
        if (m_Materials[i].AmbientColor != glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)) {
            return m_Materials[i];
        }
    }

    return m_Materials[0];
}