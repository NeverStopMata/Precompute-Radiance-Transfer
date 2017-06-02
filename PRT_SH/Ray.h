#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "Triangle.h"
using namespace glm;
using namespace std;
class Ray
{
public:
	vec3 source;
	vec3 direction;
public:
	Ray(vec3 vertexPos,vec3 direction, vec3 normal);
	~Ray();
	int IsBlocked(vector<Triangle> trianleList, int lastBlockNum);

};

