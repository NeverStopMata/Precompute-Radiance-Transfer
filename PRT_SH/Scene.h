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
#include "CoeffsVector16.h"
#include "CoeffsMat.h"
using namespace glm;
using namespace std;
class Scene
{
public:
	int numVertices;
	int numVolVertices;
	vector<vec3> volume_vertices;
	vector<vec3> vertices;
	vector<vec2> uvs;
	vector<vec3> normals;
	vector<CoeffsVector16> coeffsVecList;
	vector<CoeffsMat> coeffsMatList;
	vector<unsigned short> indices;
	vector<vec3> indexed_vertices;
	vector<vec2> indexed_uvs;
	vector<vec3> indexed_normals;
	
	vector<CoeffsVector16> indexed_coeffsVecList;
	
public:
	Scene(const char * filePath, SampleSet Ss);
	~Scene();
	void Scene::SubFacesGenerate(stack<Triangle> stack_triangles);
	void Scene::AddTiangle(Triangle newTriangle);
	void Scene::GenerateDirectCoeffs(SampleSet sampleset);
	void Scene::GenerateDirectCoeffs_CL(SampleSet sampleset);
	void Scene::GenerateTransferMatrix_CL(SampleSet sampleset);
	void Scene::ExportAllTransMats(float * target);
	vector<Triangle> Scene::GetVolumeTriangleList();
};

