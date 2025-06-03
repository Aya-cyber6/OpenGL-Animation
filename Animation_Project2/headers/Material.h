#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <glm/glm.hpp>
#include <gli/gli.hpp>
#include <stb_image.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
// Helper function to check OpenGL version (implement outside)
bool IsGLVersionHigher(int major, int minor);

struct Texture {
    unsigned int id = 0;           // OpenGL texture object ID
    GLenum m_textureTarget = GL_TEXTURE_2D; // Texture target (usually GL_TEXTURE_2D)
    unsigned int m_textureObj = 0; // Alias for id for clarity in binding

    std::string type;
    std::string path;

    // Added members
    int imageWidth = 0;
    int imageHeight = 0;
    int imageBPP = 0;
    bool isKTX = false;
    gli::gl::format ktxFormat;

    Texture(GLenum target, const char* p) : m_textureTarget(target), path(p) {
        // Initially, no texture object created (id=0)
        m_textureObj = 0;
    }

    void BindInternalNonDSA(GLenum TextureUnit)
    {
        glActiveTexture(TextureUnit); // TextureUnit is GL_TEXTURE0 + unitIndex
        glBindTexture(m_textureTarget, m_textureObj);
    }

    void BindInternalDSA(GLenum TextureUnit)
    {
        // Pass the unit index to glBindTextureUnit
        glBindTextureUnit(TextureUnit - GL_TEXTURE0, m_textureObj);
    }

    void Bind(GLenum TextureUnit)
    {
        if (IsGLVersionHigher(4, 5)) {
            BindInternalDSA(TextureUnit);
        }
        else {
            BindInternalNonDSA(TextureUnit);
        }
    }

    // Loads texture from disk; returns true if successful
    bool Load() {
        const char* pExt = strrchr(path.c_str(), '.');
        isKTX = pExt && !strcmp(pExt, ".ktx");

        unsigned char* pImageData = nullptr;

        if (isKTX) {
            gli::texture gliTex = gli::load_ktx(path.c_str());
            if (gliTex.empty()) {
                printf("Failed to load KTX texture: %s\n", path.c_str());
                return false;
            }
            gli::gl GL(gli::gl::PROFILE_KTX);
            ktxFormat = GL.translate(gliTex.format(), gliTex.swizzles());
            glm::tvec3<GLsizei> extent(gliTex.extent(0));
            imageWidth = extent.x;
            imageHeight = extent.y;
            // imageBPP is bytes per pixel, but extent.z is depth, so better to set manually
            imageBPP = 4; // or appropriate value for KTX texture
            pImageData = (unsigned char*)gliTex.data();
        }
        else {
            stbi_set_flip_vertically_on_load(1);
            pImageData = stbi_load(path.c_str(), &imageWidth, &imageHeight, &imageBPP, 0);
            if (!pImageData) {
                printf("Can't load texture from '%s' - %s\n", path.c_str(), stbi_failure_reason());
                return false;
            }
        }

        printf("Loaded texture: %s (Width: %d, Height: %d, BPP: %d)\n", path.c_str(), imageWidth, imageHeight, imageBPP);

        LoadInternal(pImageData);

        if (!isKTX && pImageData) {
            stbi_image_free(pImageData);
        }

        return true;
    }

private:
    void LoadInternal(unsigned char* pData) {
        // Generate texture object if needed
        if (m_textureObj == 0) {
            glGenTextures(1, &m_textureObj);
            id = m_textureObj; // Keep id and m_textureObj synced
        }
        glBindTexture(m_textureTarget, m_textureObj);

        if (isKTX) {
            // Upload KTX texture data
            glTexImage2D(m_textureTarget, 0,
                ktxFormat.Internal,   // internal format
                imageWidth, imageHeight, 0,
                ktxFormat.External,   // format
                ktxFormat.Type,       // data type
                pData);
        }
        else {
            GLint format = (imageBPP == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(m_textureTarget, 0, format, imageWidth, imageHeight, 0, format, GL_UNSIGNED_BYTE, pData);
        }

        // Texture parameters
        glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glGenerateMipmap(m_textureTarget);

        glBindTexture(m_textureTarget, 0);
    }
};


// Physically-Based Rendering (PBR) material properties
struct PBRMaterial {
    glm::vec3 albedo = glm::vec3(1.0f);  // Base color
    float     metallic = 0.0f;             // How metal-like the surface is

    float     roughness = 1.0f;             // Smoothness (0.0 = smooth, 1.0 = rough)
    float     ao = 1.0f;             // Ambient occlusion strength

    glm::vec3 emissive = glm::vec3(0.0f);  // Emission color
    float     padding = 0.0f;             // Padding for alignment if needed
};

// Full material definition
struct Material {
    std::string name;

    // Legacy Phong model support (optional for PBR)
    glm::vec4 AmbientColor = glm::vec4(0.0f);
    glm::vec4 DiffuseColor = glm::vec4(0.0f);
    glm::vec4 SpecularColor = glm::vec4(0.0f);

    // Transparency and alpha testing
    float transparency = 1.0f;
    float alphaTest = 0.0f;

    // PBR material parameters
    PBRMaterial pbr;

    // Optional textures
    Texture* diffuseMap = nullptr;
    Texture* normalMap = nullptr;
    Texture* metallicRoughnessMap = nullptr;
    Texture* aoMap = nullptr;
    Texture* emissiveMap = nullptr;
    Texture* pSpecularExponent = NULL;


    // Destructor to manage texture cleanup
    ~Material() {
        delete diffuseMap;
        delete normalMap;
        delete metallicRoughnessMap;
        delete aoMap;
        delete emissiveMap;
    }
};

// Rest of your structs (PBRMaterial, Material) remain unchanged

#endif // MATERIAL_H
