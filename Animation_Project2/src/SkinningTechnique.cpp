
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "..//headers/SkinningTechnique.h"
#include "..//headers/Material.h"

SkinningTechnique::SkinningTechnique() {}

bool SkinningTechnique::Init() {

    if (!Technique::Init()) {
        return false;
    }
    if (!AddShader(GL_VERTEX_SHADER, "shaders/skinning.vs")) {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "shaders/skinning.fs")) {
        return false;
    }

    if (!Finalize()) {  // After all the shaders have been added to the program call this function
        // to link and validate the program.
        return false;
    }

    // Assume shader program is compiled and linked at this point
    WVPLoc = GetUniformLocation("gWVP");
    samplerLoc = GetUniformLocation("gSampler");
    samplerSpecularExponentLoc = GetUniformLocation("gSamplerSpecularExponent");
    CameraLocalPosLoc = GetUniformLocation("gCameraLocalPos");
    NumPointLightsLocation = GetUniformLocation("gNumPointLights");
    NumSpotLightsLocation = GetUniformLocation("gNumSpotLights");

    // Material
    materialLoc.AmbientColor = GetUniformLocation("gMaterial.AmbientColor");
    materialLoc.DiffuseColor = GetUniformLocation("gMaterial.DiffuseColor");
    materialLoc.SpecularColor = GetUniformLocation("gMaterial.SpecularColor");

    // Directional Light
    dirLightLoc.Color = GetUniformLocation("gDirectionalLight.Base.Color");
    dirLightLoc.AmbientIntensity = GetUniformLocation("gDirectionalLight.Base.AmbientIntensity");
    dirLightLoc.Direction = GetUniformLocation("gDirectionalLight.Direction");
    dirLightLoc.DiffuseIntensity = GetUniformLocation("gDirectionalLight.Base.DiffuseIntensity");

    // Point Lights
    char name[128];
    for (unsigned int i = 0; i < MAX_POINT_LIGHTS; i++) {
        snprintf(name, sizeof(name), "gPointLights[%d].Base.Color", i);
        PointLightsLocation[i].Color = GetUniformLocation(name);

        snprintf(name, sizeof(name), "gPointLights[%d].Base.AmbientIntensity", i);
        PointLightsLocation[i].AmbientIntensity = GetUniformLocation(name);

        snprintf(name, sizeof(name), "gPointLights[%d].LocalPos", i);
        PointLightsLocation[i].Position = GetUniformLocation(name);

        snprintf(name, sizeof(name), "gPointLights[%d].Base.DiffuseIntensity", i);
        PointLightsLocation[i].DiffuseIntensity = GetUniformLocation(name);

        snprintf(name, sizeof(name), "gPointLights[%d].Atten.Constant", i);
        PointLightsLocation[i].Atten.Constant = GetUniformLocation(name);

        snprintf(name, sizeof(name), "gPointLights[%d].Atten.Linear", i);
        PointLightsLocation[i].Atten.Linear = GetUniformLocation(name);

        snprintf(name, sizeof(name), "gPointLights[%d].Atten.Exp", i);
        PointLightsLocation[i].Atten.Exp = GetUniformLocation(name);
    }

    // Spot Lights
    for (unsigned int i = 0; i < MAX_SPOT_LIGHTS; i++) {
        snprintf(name, sizeof(name), "gSpotLights[%d].Base.Base.Color", i);
        SpotLightsLocation[i].Color = GetUniformLocation(name);

        snprintf(name, sizeof(name), "gSpotLights[%d].Base.Base.AmbientIntensity", i);
        SpotLightsLocation[i].AmbientIntensity = GetUniformLocation(name);

        snprintf(name, sizeof(name), "gSpotLights[%d].Base.LocalPos", i);
        SpotLightsLocation[i].Position = GetUniformLocation(name);

        snprintf(name, sizeof(name), "gSpotLights[%d].Base.Base.DiffuseIntensity", i);
        SpotLightsLocation[i].DiffuseIntensity = GetUniformLocation(name);

        snprintf(name, sizeof(name), "gSpotLights[%d].Base.Atten.Constant", i);
        SpotLightsLocation[i].Atten.Constant = GetUniformLocation(name);

        snprintf(name, sizeof(name), "gSpotLights[%d].Base.Atten.Linear", i);
        SpotLightsLocation[i].Atten.Linear = GetUniformLocation(name);

        snprintf(name, sizeof(name), "gSpotLights[%d].Base.Atten.Exp", i);
        SpotLightsLocation[i].Atten.Exp = GetUniformLocation(name);

        snprintf(name, sizeof(name), "gSpotLights[%d].Direction", i);
        SpotLightsLocation[i].Direction = GetUniformLocation(name);

        snprintf(name, sizeof(name), "gSpotLights[%d].Cutoff", i);
        SpotLightsLocation[i].Cutoff = GetUniformLocation(name);
    }

    displayBoneIndexLocation = GetUniformLocation("gDisplayBoneIndex");

    return true;
}

void SkinningTechnique::SetWVP(const glm::mat4& WVP) {
    glUniformMatrix4fv(WVPLoc, 1, GL_FALSE, glm::value_ptr(WVP));
}

void SkinningTechnique::SetTextureUnit(unsigned int TextureUnit) {
    glUniform1i(samplerLoc, TextureUnit);
}

void SkinningTechnique::SetSpecularExponentTextureUnit(unsigned int TextureUnit) {
    glUniform1i(samplerSpecularExponentLoc, TextureUnit);
}

void SkinningTechnique::SetCameraLocalPos(const glm::vec3& CameraLocalPos) {
    glUniform3fv(CameraLocalPosLoc, 1, glm::value_ptr(CameraLocalPos));
}

void SkinningTechnique::SetMaterial(const Material& material) {
    glUniform3fv(materialLoc.AmbientColor, 1, glm::value_ptr(material.AmbientColor));
    glUniform3fv(materialLoc.DiffuseColor, 1, glm::value_ptr(material.DiffuseColor));
    glUniform3fv(materialLoc.SpecularColor, 1, glm::value_ptr(material.SpecularColor));
}

void SkinningTechnique::SetDirectionalLight(const DirectionalLight& Light) {
    glUniform3fv(dirLightLoc.Color, 1, glm::value_ptr(Light.Color));
    glUniform1f(dirLightLoc.AmbientIntensity, Light.AmbientIntensity);
    glUniform3fv(dirLightLoc.Direction, 1, glm::value_ptr(Light.LocalDirection));
    glUniform1f(dirLightLoc.DiffuseIntensity, Light.DiffuseIntensity);
}

void SkinningTechnique::SetPointLights(unsigned int NumLights, const PointLight* pLights) {
    glUniform1i(NumPointLightsLocation, NumLights);

    for (unsigned int i = 0; i < NumLights; ++i) {
        glUniform3fv(PointLightsLocation[i].Color, 1, glm::value_ptr(pLights[i].Color));
        glUniform1f(PointLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform3fv(PointLightsLocation[i].Position, 1, glm::value_ptr(pLights[i].LocalPosition));
        glUniform1f(PointLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
        glUniform1f(PointLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(PointLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
        glUniform1f(PointLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
    }
}

void SkinningTechnique::SetSpotLights(unsigned int NumLights, const SpotLight* pLights) {
    glUniform1i(NumSpotLightsLocation, NumLights);

    for (unsigned int i = 0; i < NumLights; ++i) {
        glUniform3fv(SpotLightsLocation[i].Color, 1, glm::value_ptr(pLights[i].Color));
        glUniform1f(SpotLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform3fv(SpotLightsLocation[i].Position, 1, glm::value_ptr(pLights[i].LocalPosition));
        glUniform1f(SpotLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
        glUniform3fv(SpotLightsLocation[i].Direction, 1, glm::value_ptr(pLights[i].LocalDirection));
        glUniform1f(SpotLightsLocation[i].Cutoff, pLights[i].Cutoff);
        glUniform1f(SpotLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(SpotLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
        glUniform1f(SpotLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
    }
}

void SkinningTechnique::SetDisplayBoneIndex(uint DisplayBoneIndex) {
    glUniform1i(displayBoneIndexLocation, DisplayBoneIndex);
}
