#include "CoeffsVector16.h"



CoeffsVector16::CoeffsVector16()
{
	for (int i = 0; i < 16; i++)
		Array[i] = 0.0f;
}


CoeffsVector16::~CoeffsVector16()
{
}

CoeffsVector16 CoeffsVector16::rotate(float theta, float phi)
{
	SH_RotationManager sh_rotator;
	CoeffsVector16 newCoeffsVec;
	sh_rotator.RotateSHCoefficients(4, this->Array, newCoeffsVec.Array, theta, phi);
	return newCoeffsVec;
}