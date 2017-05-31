#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "SampleSet.h"
#include "SH_RotationManager.h"
using namespace glm;
class Light
{
public:
	vec3 direction;
	float size;
	float * coeffs;
	float * rotatedCoeffs;
	int numBands;
public:
	Light();
	Light(float size, SampleSet Ss);
	~Light();
	float IsInLightArea(float theta);
	void  RotateLight(float theta, float phi);
	mat4 getRotatedCoeffsMatrix();
};

