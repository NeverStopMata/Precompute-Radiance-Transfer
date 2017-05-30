#pragma once
#include <vector>
#include <stack>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Triangle.h"
using namespace glm;
using namespace std;
class Scene
{
public:
	vector<vec3> volume_vertices;
	vector<vec3> vertices;
	vector<vec2> uvs;
	vector<vec3> normals;
	vector<unsigned short> indices;
	vector<vec3> indexed_vertices;
	vector<vec2> indexed_uvs;
	vector<vec3> indexed_normals;
public:
	Scene(const char * filePath);
	~Scene();
	void Scene::SubFacesGenerate(stack<Triangle> stack_triangles);
	void Scene::AddTiangle(Triangle newTriangle);
};

