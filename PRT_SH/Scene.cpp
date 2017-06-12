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

	//GenerateDirectCoeffs(Ss);
	GenerateDirectCoeffs_CL(Ss);
	GenerateTransferMatrix(Ss);
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
	vector<float *> coeffsList;
	vector<float *> transferMatrix;
	vector<unsigned short> indices;
	vector<vec3> indexed_vertices;
	vector<vec2> indexed_uvs;
	vector<vec3> indexed_normals;
	vector<float *> indexed_coeffsList;
	for each (float * var in coeffsList)
	{
		if (var)
			delete[] var;
		var = NULL;
	}
	for each (float * var in transferMatrix)
	{
		if (var)
			delete[] var;
		var = NULL;
	}
	for each (float * var in indexed_coeffsList)
	{
		if (var)
			delete[] var;
		var = NULL;
	}
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
				/*for (int k = 0; k < numFunctions; ++k)
				{
					float contribution = dotResult * sampleset.all[j].shValues[k];
					Coeffs[k] += contribution;
				}*/
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

void Scene::GenerateDirectCoeffs_CL(SampleSet sampleset)
{
	OpenCL_Math CLtool;
	const int numDots = vertices.size();
	const int numVertices = volume_vertices.size();
	float* sampleDircts = new float[sampleset.numSamples * 3];
	for (int i = 0; i < sampleset.numSamples *3; i++)
	{
		sampleDircts[i] = sampleset.all[i / 3].direction[i % 3];
	}
	float* dotNormals = new float[numDots * 3];
	for (int i = 0; i < numDots * 3; i++)
	{
		dotNormals[i] = normals[i / 3][i % 3];
	}
	float* dotPos = new float[numDots * 3];
	for (int i = 0; i < numDots * 3; i++)
	{
		dotPos[i] = vertices[i / 3][i % 3];
	}
	float* vertexPos = new float[numVertices * 3];
	for (int i = 0; i < numVertices * 3; i++)
	{
		vertexPos[i] = volume_vertices[i / 3][i % 3];
	}
	float* faceNromals = new float[numVertices];
	vector<Triangle> triangleList4BlockDetect = this->GetVolumeTriangleList();
	for (int i = 0; i < numVertices; i++)
	{
		faceNromals[i] = triangleList4BlockDetect[i / 3].faceNormal[i % 3];
	}
	int numDots_round;
	numDots_round = (numDots / 16 + 1)*16;
	float* sceneMatrix = new float[sampleset.numSamples * numDots_round];
	for (int i = sampleset.numSamples * numDots; i < sampleset.numSamples * numDots_round; i++)
		sceneMatrix[i] = 0.0f;
	CLtool.GetSceneMatrix(sampleset.numSamples, numDots, numVertices, sampleDircts, dotNormals, dotPos, vertexPos, faceNromals, sceneMatrix);
	int numSH_Mat = sampleset.numSamples * sampleset.numBands * sampleset.numBands;
	float * SH_BasicMat = new float[numSH_Mat];
	for (int i = 0; i < numSH_Mat; i++)
	{
		SH_BasicMat[i] = sampleset.all[i/16].shValues[i%16];
	}
	float * coeffMat = new float[sampleset.numBands * sampleset.numBands * numDots_round];
	CLtool.MatMultip(16, numDots_round, sampleset.numSamples, SH_BasicMat, sceneMatrix, coeffMat);
	for (int i = 0; i < numDots; i++)
	{
		coeffsList.push_back(&coeffMat[i * sampleset.numBands * sampleset.numBands]);
	}
	delete[] sampleDircts;
	delete[] dotNormals;
	delete[] dotPos;
	delete[] vertexPos;
	delete[] faceNromals;
	delete[] sceneMatrix;
	delete[] SH_BasicMat;
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
void Scene::GenerateTransferMatrix(SampleSet sampleset)
{
	const int numFunctions = sampleset.numBands * sampleset.numBands;
	const int numEntries = numFunctions * numFunctions;
	//Create space for the SH coefficients in each vertex
	const int numVertices = vertices.size();
	for (int i = 0; i < numVertices; ++i)
	{
		float * transferMatrix = new float[numEntries];
		for (int j = 0; j < numEntries; j++)
		{
			transferMatrix[j] = 0.0f;
		}
		for (int k = 0; k < numFunctions; k++)
			for (int l = 0; l < numFunctions; l++)
			{

			}
		transferMatrixs.push_back(transferMatrix);
	}
}