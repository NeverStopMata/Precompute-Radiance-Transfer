#include "Light.h"



Light::Light()
{
}
Light::Light(float size, SampleSet Ss)
{
	int numFunctions = Ss.numBands * Ss.numBands;
	numBands = Ss.numBands;
	this->size = size;
	for (int i = 0; i < numFunctions; i++)
	{
		unrotatedCoeffs.Array[i] = 0.0f;
		for (int j = 0; j < Ss.numSamples; j++)
		{
			unrotatedCoeffs.Array[i] += IsInLightArea(Ss.all[j].theta) * Ss.all[j].shValues.Array[i];
			
		}
		unrotatedCoeffs.Array[i] *= 4.0f * M_PI / Ss.numSamples;
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
	rotatedCoeffs = unrotatedCoeffs.rotate(theta, phi);
}

mat4 Light::getRotatedCoeffsMatrix()
{
	return this->rotatedCoeffs.GetMatrix4X4();
}