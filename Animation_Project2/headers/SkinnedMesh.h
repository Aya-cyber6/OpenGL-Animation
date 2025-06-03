#ifndef SKINNED_MESH_H
#define SKINNED_MESH_H

#include <glad/glad.h>

#include <map>
#include <vector>
#include <string>
#include <cassert>
#include <cstdint>

#include <glm/glm.hpp>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "Material.h"

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

class SkinnedMesh
{
public:
    SkinnedMesh() {};

    bool LoadMesh(const std::string& Filename);
    void Render();

    unsigned int NumBones() const
    {
        return static_cast<unsigned int>(m_BoneNameToIndexMap.size());
    }

    // You need to define your own transform & material types or integrate here
    // For now, just placeholders:
     glm::mat4& GetWorldTransform();
     const Material& GetMaterial();

private:
    static constexpr int MAX_NUM_BONES_PER_VERTEX = 4;

    void Clear();

    bool InitFromScene(const aiScene* pScene, const std::string& Filename);

    void CountVerticesAndIndices(const aiScene* pScene, unsigned int& NumVertices, unsigned int& NumIndices);
    void ReserveSpace(unsigned int NumVertices, unsigned int NumIndices);
    void InitAllMeshes(const aiScene* pScene);
    void InitSingleMesh(unsigned int MeshIndex, const aiMesh* paiMesh);
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void PopulateBuffers();
    void LoadTextures(const std::string& Dir, const aiMaterial* pMaterial, int index);
    void LoadDiffuseTexture(const std::string& Dir, const aiMaterial* pMaterial, int index);
    void LoadSpecularTexture(const std::string& Dir, const aiMaterial* pMaterial, int index);
    void LoadColors(const aiMaterial* pMaterial, int index);

    // Texture loading functions omitted, implement as needed or integrate with your own texture loader

    struct VertexBoneData
    {
        unsigned int BoneIDs[MAX_NUM_BONES_PER_VERTEX] = { 0 };
        float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f };

        VertexBoneData() = default;

        void AddBoneData(unsigned int BoneID, float Weight)
        {
            for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(BoneIDs); i++) {
                if (Weights[i] == 0.0f) {
                    BoneIDs[i] = BoneID;
                    Weights[i] = Weight;
                    return;
                }
            }
            assert(false && "More bones than allowed per vertex");
        }
    };

    void LoadMeshBones(unsigned int MeshIndex, const aiMesh* paiMesh);
    void LoadSingleBone(unsigned int MeshIndex, const aiBone* pBone);
    int GetBoneId(const aiBone* pBone);

    static constexpr unsigned int INVALID_MATERIAL = 0xFFFFFFFF;

    enum BUFFER_TYPE {
        INDEX_BUFFER = 0,
        POS_VB = 1,
        TEXCOORD_VB = 2,
        NORMAL_VB = 3,
        BONE_VB = 4,
        NUM_BUFFERS = 5
    };

    // Replace or implement your own transform type
    glm::mat4 m_worldTransform;
    GLuint m_VAO = 0;
    GLuint m_Buffers[NUM_BUFFERS] = { 0 };

    struct BasicMeshEntry {
        BasicMeshEntry()
            : NumIndices(0),
            BaseVertex(0),
            BaseIndex(0),
            MaterialIndex(INVALID_MATERIAL)
        {
        }

        unsigned int NumIndices;
        unsigned int BaseVertex;
        unsigned int BaseIndex;
        unsigned int MaterialIndex;
    };

    std::vector<BasicMeshEntry> m_Meshes;
    std::vector<Material> m_Materials;

    // Temporary space for vertex data before loading into GPU
    std::vector<glm::vec3> m_Positions;
    std::vector<glm::vec3> m_Normals;
    std::vector<glm::vec2> m_TexCoords;
    std::vector<unsigned int> m_Indices;
    std::vector<VertexBoneData> m_Bones;

    std::map<std::string, unsigned int> m_BoneNameToIndexMap;
};

#endif  /* SKINNED_MESH_H */
