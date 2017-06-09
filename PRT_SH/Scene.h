#pragma once
#include <vector>
#include <stack>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <CL\cl.h>
#include "Triangle.h"
#include "SampleSet.h"
#include "Ray.h"
#include "OpenCL_Math.h"
using namespace glm;
using namespace std;
class Scene
{
public:
	vector<vec3> volume_vertices;
	vector<vec3> vertices;
	vector<vec2> uvs;
	vector<vec3> normals;
	vector<float *> coeffsList;
	vector<float *> transferMatrixs;
	vector<unsigned short> indices;
	vector<vec3> indexed_vertices;
	vector<vec2> indexed_uvs;
	vector<vec3> indexed_normals;
	
	vector<float *> indexed_coeffsList;
	
public:
	Scene(const char * filePath, SampleSet Ss);
	~Scene();
	void Scene::SubFacesGenerate(stack<Triangle> stack_triangles);
	void Scene::AddTiangle(Triangle newTriangle);
	void Scene::GenerateDirectCoeffs(SampleSet sampleset);
	void Scene::GenerateDirectCoeffs_CL(SampleSet sampleset);
	void Scene::GenerateTransferMatrix(SampleSet sampleset);
	vector<Triangle> Scene::GetVolumeTriangleList();
};

