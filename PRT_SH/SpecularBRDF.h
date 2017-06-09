#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "SampleSet.h"
using namespace glm;
using namespace std;
class SpecularBRDF
{
public:
	SpecularBRDF::SpecularBRDF(SampleSet Ss);
	~SpecularBRDF();
	mat4 SpecularBRDF::getInRadianceCoeffsMatrix();
	float SpecularBRDF::ReflectRate(float theta);
public:
	float * inRadCoeffs;
	int numBands;
};

