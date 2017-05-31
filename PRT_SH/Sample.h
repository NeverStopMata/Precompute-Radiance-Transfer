#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
	float * shValues;
public:
	Sample();
	~Sample();
};

