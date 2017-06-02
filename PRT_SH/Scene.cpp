#include "Scene.h"

#include "common/objloader.hpp"
#include "common/vboindexer.hpp"
#include <iostream>
using namespace std;
using namespace glm;
Scene::Scene(const char * filePath,SampleSet Ss)
{
	vector<vec3> temp_vertices;
	vector<vec2> temp_uvs;
	vector<vec3> temp_normals;
	stack<Triangle> stack_triangles;
	bool res = loadOBJ(filePath, temp_vertices, temp_uvs, temp_normals);
	for (int i = 0; i < temp_vertices.size(); i += 3)
	{
		Triangle triangle;
		for (int j = 0; j < 3; j++)
		{
			triangle.vertices[j] = temp_vertices[i + j];
			triangle.uvs[j] = temp_uvs[i + j];
			triangle.normals[j] = temp_normals[i + j];
		}
		stack_triangles.push(triangle);
	}
	cout << "一开始的三角形个数：" << stack_triangles.size() << endl;
	SubFacesGenerate(stack_triangles);
	volume_vertices = temp_vertices;

	GenerateDirectCoeffs(Ss);
	//indexVBO(temp_vertices, temp_uvs, temp_normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
	indexVBO(vertices, uvs, normals, coeffsList, indices, indexed_vertices, indexed_uvs, indexed_normals, indexed_coeffsList);
}

void Scene::SubFacesGenerate(stack<Triangle> stack_triangles)
{
	
	//cout << "test" << temp_vertices.size() << "/" << temp_uvs.size() << "/" << temp_normals.size() << endl;
	while (!stack_triangles.empty())
	{
		cout << stack_triangles.size() << endl;
		Triangle temp_triangle = stack_triangles.top();
		stack_triangles.pop();
		int longSideNum = temp_triangle.IsTooBig();
		if (longSideNum < 0)
		{
			AddTiangle(temp_triangle);		
		}
		else
		{
			Triangle subTriangle1, subTriangle2;
			temp_triangle.DivideTo(&subTriangle1, &subTriangle2, longSideNum);
			stack_triangles.push(subTriangle1);
			stack_triangles.push(subTriangle2);
			float ab, bc, ca;
			float ab1, bc1, ca1, ab2,bc2,ca2;
			ab = temp_triangle.GetSideLength(0);
			bc = temp_triangle.GetSideLength(1);
			ca = temp_triangle.GetSideLength(2);
			ab1 = subTriangle1.GetSideLength(0);
			bc1 = subTriangle1.GetSideLength(1);
			ca1 = subTriangle1.GetSideLength(2);
			ab2 = subTriangle2.GetSideLength(0);
			bc2 = subTriangle2.GetSideLength(1);
			ca2 = subTriangle2.GetSideLength(2);


		}
	}
    
}


void Scene::AddTiangle(Triangle newTriangle)
{
	for (int i = 0; i < 3; i++)
	{
		vertices.push_back(newTriangle.vertices[i]);
		uvs.push_back(newTriangle.uvs[i]);
		normals.push_back(newTriangle.normals[i]);
	}	
}
Scene::~Scene()
{
}


void Scene::GenerateDirectCoeffs(SampleSet sampleset)
{
	const int numFunctions = sampleset.numBands * sampleset.numBands;

	//Create space for the SH coefficients in each vertex
	const int numVertices = vertices.size();


	vector<Triangle> triangleList4BlockDetect = this->GetVolumeTriangleList();
	for (int i = 0; i < numVertices; ++i)
	{
		float * Coeffs = new float[numFunctions];
		for (int j = 0; j < numFunctions; j++)
		{
			Coeffs[j] = 0.0f;
		}
		int lastBlockFaceNum = 0;
		int blockNum = 0;
		for (int j = 0; j < sampleset.numSamples; j++)
		{
			float dotResult = dot(sampleset.all[j].direction, normals[i]);
			
			if (dotResult > 0.0f)
			{
				Ray currentRay(this->vertices[i], sampleset.all[j].direction, normals[i]);
				if ((lastBlockFaceNum = currentRay.IsBlocked(triangleList4BlockDetect, lastBlockFaceNum)) < 0)
				{
					for (int k = 0; k < numFunctions; ++k)
					{
						float contribution = dotResult * sampleset.all[j].shValues[k];
						Coeffs[k] += contribution;
					}
					lastBlockFaceNum = 0;
				}	
			}	
		}
		for (int j = 0; j < numFunctions; ++j)
		{
			Coeffs[j] *= 4 * M_PI / sampleset.numSamples;
		}
		this->coeffsList.push_back(Coeffs);
		cout <<"current vertex: NO. "<< i <<"  "<<"blocked rays number:"<<blockNum<< endl;
	}
}

vector<Triangle> Scene::GetVolumeTriangleList()
{
	vector<Triangle> resTrgleList;
	for (int i = 0; i < this->volume_vertices.size() / 3; i++)
	{
		Triangle newOne(volume_vertices[i * 3], volume_vertices[i * 3 + 1], volume_vertices[i * 3 + 2]);
		resTrgleList.push_back(newOne);
	}
	return resTrgleList;
}