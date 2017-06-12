#pragma once
#include "SH_RotationManager.h"
class CoeffsVector16
{
public:
	CoeffsVector16();
	~CoeffsVector16();
	CoeffsVector16 CoeffsVector16::rotate(float theta, float phi);
public:
	float Array[16];

};

