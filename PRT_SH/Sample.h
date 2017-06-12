#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "CoeffsVector16.h"
using namespace glm;
class Sample
{
public:
	//Spherical polar coords
	float theta;
	float phi;

	//Cartesian direction
	vec3 direction;

	//Values of each SH function at this point
	CoeffsVector16 shValues;
public:
	Sample();
	~Sample();
};

