#ifndef SKINNING_TECHNIQUE_H
#define SKINNING_TECHNIQUE_H

#include "technique.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "..//headers/Material.h"

typedef unsigned int uint;

class BaseLight {
public:
    glm::vec3 Color = glm::vec3(1.0f);
    float AmbientIntensity = 0.0f;
    float DiffuseIntensity = 0.0f;
};

class DirectionalLight : public BaseLight {
public:
    glm::vec3 WorldDirection = glm::vec3(0.0f);
    glm::vec3 LocalDirection = glm::vec3(0.0f);

    void CalcLocalDirection(const glm::mat4& modelMatrix) {
        LocalDirection = glm::mat3(modelMatrix) * WorldDirection;
    }
};

struct LightAttenuation {
    float Constant = 1.0f;
    float Linear = 0.0f;
    float Exp = 0.0f;
};

class PointLight : public BaseLight {
public:
    glm::vec3 WorldPosition = glm::vec3(0.0f);
    glm::vec3 LocalPosition = glm::vec3(0.0f);
    LightAttenuation Attenuation;

    void CalcLocalPosition(const glm::mat4& modelMatrix) {
        LocalPosition = glm::vec3(modelMatrix * glm::vec4(WorldPosition, 1.0));
    }
};

class SpotLight : public PointLight {
public:
    glm::vec3 WorldDirection = glm::vec3(0.0f);
    glm::vec3 LocalDirection = glm::vec3(0.0f);
    float Cutoff = 0.0f;

    void CalcLocalDirectionAndPosition(const glm::mat4& modelMatrix) {
        CalcLocalPosition(modelMatrix);
        LocalDirection = glm::mat3(modelMatrix) * WorldDirection;
    }
};

class SkinningTechnique : public Technique {
public:
    static const unsigned int MAX_POINT_LIGHTS = 2;
    static const unsigned int MAX_SPOT_LIGHTS = 2;

    SkinningTechnique();
    virtual bool Init();

    void SetWVP(const glm::mat4& WVP);
    void SetTextureUnit(unsigned int TextureUnit);
    void SetSpecularExponentTextureUnit(unsigned int TextureUnit);
    void SetDirectionalLight(const DirectionalLight& Light);
    void SetPointLights(unsigned int NumLights, const PointLight* pLights);
    void SetSpotLights(unsigned int NumLights, const SpotLight* pLights);
    void SetCameraLocalPos(const glm::vec3& CameraLocalPos);
    void SetMaterial(const Material& material);
    void SetDisplayBoneIndex(uint DisplayBoneIndex);

private:
    GLuint WVPLoc;
    GLuint samplerLoc;
    GLuint samplerSpecularExponentLoc;
    GLuint CameraLocalPosLoc;
    GLuint NumPointLightsLocation;
    GLuint NumSpotLightsLocation;

    struct {
        GLuint AmbientColor;
        GLuint DiffuseColor;
        GLuint SpecularColor;
    } materialLoc;

    struct {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint Direction;
        GLuint DiffuseIntensity;
    } dirLightLoc;

    struct {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint Position;
        GLuint DiffuseIntensity;
        struct {
            GLuint Constant;
            GLuint Linear;
            GLuint Exp;
        } Atten;
    } PointLightsLocation[MAX_POINT_LIGHTS];

    struct {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint DiffuseIntensity;
        GLuint Position;
        GLuint Direction;
        GLuint Cutoff;
        struct {
            GLuint Constant;
            GLuint Linear;
            GLuint Exp;
        } Atten;
    } SpotLightsLocation[MAX_SPOT_LIGHTS];

    GLuint displayBoneIndexLocation;
};

#endif  // SKINNING_TECHNIQUE_H
