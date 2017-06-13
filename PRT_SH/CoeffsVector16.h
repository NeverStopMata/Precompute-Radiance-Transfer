#pragma once
#include "SH_RotationManager.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
class CoeffsVector16
{
public:
	CoeffsVector16();
	~CoeffsVector16();
	CoeffsVector16 CoeffsVector16::rotate(float theta, float phi);
	mat4 CoeffsVector16::GetMatrix4X4();
public:
	float Array[16];

};

