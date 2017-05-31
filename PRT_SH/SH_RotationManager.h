#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
class SH_RotationManager
{
public:
	SH_RotationManager();
	~SH_RotationManager();
	void RotateSHCoefficients(int numBands, float * unrotatedCoeffs, float * rotatedCoeffs,
		float theta, float phi);
	void GetZRotationMatrix(int band, float * entries, float angle);
	void GetX90DegreeRotationMatrix(int band, float * entries);
	void ApplyMatrix(int size, float * matrix, bool transpose,float * inVector, float * outVector);
};

