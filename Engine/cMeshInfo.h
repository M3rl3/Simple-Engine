#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>

#include "cVAOManager/cVAOManager.h"

class cMeshInfo {

public:

	cMeshInfo();
	~cMeshInfo();

	std::string meshName;
	std::string friendlyName;

	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 velocity;
	glm::vec3 target;
	glm::quat rotation;
	glm::vec4 colour;
	glm::vec4 RGBAColour;
	
	bool isWireframe;
	bool isVisible;
	bool useRGBAColour;
	bool hasTexture;
	bool doNotLight;
	bool isTerrainMesh;
	bool isSkyBoxMesh;
	bool hasChildMeshes;

	std::string textures[8];
	float textureRatios[8];

	glm::vec3 min;
	glm::vec3 max;

	void SetRotationFromEuler(glm::vec3 newEulerAngleXYZ);
	void AdjustRoationAngleFromEuler(glm::vec3 EulerAngleXYZ_Adjust);
	void SetUniformScale(float newScale);
	void SetTextureRatiosAcrossTheBoard(float newTextureRatio);

	std::vector <cMeshInfo*> vecChildMeshes;
};