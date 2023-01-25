#include "cMeshInfo.h"

cMeshInfo::cMeshInfo() {

	this->position = glm::vec3(0.f);
	this->rotation = glm::quat(glm::vec3(0.f));
	this->colour = glm::vec4(0.f, 0.f, 0.f, 1.f);
	this->RGBAColour = glm::vec4(0.f, 0.f, 0.f, 1.f);
	this->scale = glm::vec3(1.f);
	this->isWireframe = false;
	this->isVisible = true;
	this->useRGBAColour = false;
	this->doNotLight = false;
	this->isTerrainMesh = false;
	this->hasTexture = false;
	this->isSkyBoxMesh = false;
	this->hasChildMeshes = false;
	this->min = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	this->max = glm::vec3(FLT_MIN, FLT_MIN, FLT_MIN);
	this->SetTextureRatiosAcrossTheBoard(0.f);
}

cMeshInfo::~cMeshInfo() {

}

void cMeshInfo::SetRotationFromEuler(glm::vec3 newEulerAngleXYZ)
{
	this->rotation = glm::quat(newEulerAngleXYZ);
}

void cMeshInfo::AdjustRoationAngleFromEuler(glm::vec3 EulerAngleXYZ_Adjust)
{
	// To combine quaternion values, you multiply them together
		// Make a quaternion that represents that CHANGE in angle
	glm::quat qChange = glm::quat(EulerAngleXYZ_Adjust);
	// Multiply them together to get the change
	this->rotation *= qChange;

	//		// This is the same as this
	//		this->qRotation = this->qRotation * qChange;
}

void cMeshInfo::SetUniformScale(float newScale)
{
	this->scale = glm::vec3(newScale, newScale, newScale);
}

void cMeshInfo::SetTextureRatiosAcrossTheBoard(float newTextureRatio)
{
	for (int i = 0; i < 8; i++) {
		this->textureRatios[i] = newTextureRatio;
	}
}