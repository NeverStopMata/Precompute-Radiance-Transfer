#pragma once
#include "SH_Calculator.h"
#include "SampleSet.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

class BRDF_Manager
{
public:
	BRDF_Manager();
	BRDF_Manager(SampleSet Ss);
	~BRDF_Manager();
public:
	CoeffsVector16 coeffsVec;
};

