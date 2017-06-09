#include "SpecularBRDF.h"



SpecularBRDF::SpecularBRDF(SampleSet Ss)
{
	int numFunctions = Ss.numBands * Ss.numBands;
	numBands = Ss.numBands;
	inRadCoeffs = new float[numFunctions];
	for (int i = 0; i < numFunctions; i++)
	{
		inRadCoeffs[i] = 0.0f;
		for (int j = 0; j < Ss.numSamples; j++)
		{
			inRadCoeffs[i] += ReflectRate(Ss.all[j].theta) * Ss.all[j].shValues[i];

		}
		inRadCoeffs[i] *= 4.0f * M_PI / Ss.numSamples;
	}
}

SpecularBRDF::~SpecularBRDF()
{
}

mat4 SpecularBRDF::getInRadianceCoeffsMatrix()
{
	mat4 resultMatrix;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			resultMatrix[i][j] = this->inRadCoeffs[i * 4 + j];
		}
	}
	return resultMatrix;
}

float SpecularBRDF::ReflectRate(float theta)
{
	return theta < 0.5 ? 1.0 : 0.0;
}