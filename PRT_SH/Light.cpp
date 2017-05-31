#include "Light.h"



Light::Light()
{
}
Light::Light(float size, SampleSet Ss)
{
	int numFunctions = Ss.numBands * Ss.numBands;
	numBands = Ss.numBands;
	coeffs = new float[numFunctions];
	for (int i = 0; i < numFunctions; i++)
	{
		coeffs[i] = 0.0f;
		for (int j = 0; j < Ss.numSamples; j++)
		{
			coeffs[i] += IsInLightArea(Ss.all[j].theta) * Ss.all[j].shValues[i];
		}
		coeffs[i] *= 4.0f * 3.1415926f / Ss.numSamples;
	}
}


Light::~Light()
{
}

float Light::IsInLightArea(float theta)
{
	return (theta < size) ? 1.0f : 0.0f;
}

void  Light::RotateLight(float theta, float phi)
{
	SH_RotationManager SHRotManager;
	SHRotManager.RotateSHCoefficients(numBands, coeffs, rotatedCoeffs, theta, phi);
}