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
	numVolVertices = volume_vertices.size();
	numVertices = vertices.size();
	GenerateDirectCoeffs_CL(Ss);
	//GenerateTransferMatrix_CL(Ss);
	indexVBO(vertices, uvs, normals, coeffsVecList, indices, indexed_vertices, indexed_uvs, indexed_normals, indexed_coeffsVecList);
}

Scene::~Scene()
{
	volume_vertices.clear();
	vertices.clear();
	uvs.clear();
	normals.clear();
	coeffsVecList.clear();
	indices.clear();
	indexed_vertices.clear();
	indexed_uvs.clear();
	indexed_normals.clear();
	indexed_coeffsVecList.clear();
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



void Scene::GenerateDirectCoeffs(SampleSet sampleset)
{
	const int numFunctions = sampleset.numBands * sampleset.numBands;

	//Create space for the SH coefficients in each vertex
	const int numVertices = vertices.size();


	vector<Triangle> triangleList4BlockDetect = this->GetVolumeTriangleList();
	for (int i = 0; i < numVertices; ++i)
	{
		CoeffsVector16 Coeffs;
		for (int j = 0; j < numFunctions; j++)
		{
			Coeffs.Array[j] = 0.0f;
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
						float contribution = dotResult * sampleset.all[j].shValues.Array[k];
						Coeffs.Array[k] += contribution;
					}
					lastBlockFaceNum = 0;
				}	                                                                                                                                                                
			}	
		}
		for (int j = 0; j < numFunctions; ++j)
		{
			Coeffs.Array[j] *= 4 * M_PI / sampleset.numSamples;
		}
		this->coeffsVecList.push_back(Coeffs);
		cout <<"current vertex: NO. "<< i <<"  "<<"blocked rays number:"<<blockNum<< endl;
	}
}

void Scene::GenerateDirectCoeffs_CL(SampleSet sampleset)
{
	OpenCL_Math CLtool;
	float* sampleDircts = new float[sampleset.numSamples * 3];
	for (int i = 0; i < sampleset.numSamples *3; i++)
	{
		sampleDircts[i] = sampleset.all[i / 3].direction[i % 3];
	}
	float* dotNormals = new float[numVertices * 3];
	for (int i = 0; i < numVertices * 3; i++)
	{
		dotNormals[i] = normals[i / 3][i % 3];
	}
	float* dotPos = new float[numVertices * 3];
	for (int i = 0; i < numVertices * 3; i++)
	{
		dotPos[i] = vertices[i / 3][i % 3];
	}
	float* vertexPos = new float[numVolVertices * 3];
	for (int i = 0; i < numVolVertices * 3; i++)
	{
		vertexPos[i] = volume_vertices[i / 3][i % 3];
	}
	float* faceNromals = new float[numVolVertices];
	vector<Triangle> triangleList4BlockDetect = this->GetVolumeTriangleList();
	for (int i = 0; i < numVolVertices; i++)
	{
		faceNromals[i] = triangleList4BlockDetect[i / 3].faceNormal[i % 3];
	}
	int numDots_round;
	numDots_round = (numVertices / 16 + 1)*16;
	float* sceneMatrix = new float[sampleset.numSamples * numDots_round];
	for (int i = sampleset.numSamples * numVertices; i < sampleset.numSamples * numDots_round; i++)
		sceneMatrix[i] = 0.0f;
	CLtool.GetSceneMatrix(sampleset.numSamples, numVertices, numVolVertices, sampleDircts, dotNormals, dotPos, vertexPos, faceNromals, sceneMatrix);
	int numSH_Mat = sampleset.numSamples * sampleset.numBands * sampleset.numBands;
	float * SH_BasicMat = new float[numSH_Mat];
	for (int i = 0; i < numSH_Mat; i++)
	{
		SH_BasicMat[i] = sampleset.all[i/16].shValues.Array[i%16];
	}
	float * coeffMat = new float[sampleset.numBands * sampleset.numBands * numDots_round];
	CLtool.MatMultip(16, numDots_round, sampleset.numSamples, SH_BasicMat, sceneMatrix, coeffMat);
	for (int i = 0; i < numVertices; i++)
	{
		CoeffsVector16 tempCoeffs;
		for (int j = 0; j < 16; j++)
			tempCoeffs.Array[j] = coeffMat[i*sampleset.numBands * sampleset.numBands + j];
		coeffsVecList.push_back(tempCoeffs);
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
void Scene::GenerateTransferMatrix_CL(SampleSet sampleset)
{
	OpenCL_Math CLtool;
	const int numFunctions = sampleset.numBands * sampleset.numBands;
	const int numEntries = numFunctions * numFunctions;
	int numSH_Mat = sampleset.numSamples * sampleset.numBands * sampleset.numBands;
	float * SH_BasicMat = new float[numSH_Mat];
	for (int i = 0; i < numSH_Mat; i++)
	{
		SH_BasicMat[i] = sampleset.all[i / 16].shValues.Array[i % 16];
	}
	float * SH_BasicMat_T = new float[numSH_Mat];
	for (int i = 0; i < numSH_Mat; i++)
	{
		SH_BasicMat_T[i] = sampleset.all[i % sampleset.numSamples].shValues.Array[i / sampleset.numSamples];
	}

	float* sampleDircts = new float[sampleset.numSamples * 3];
	for (int i = 0; i < sampleset.numSamples * 3; i++)
	{
		sampleDircts[i] = sampleset.all[i / 3].direction[i % 3];
	}

	float* vertexPos = new float[numVolVertices * 3];
	for (int i = 0; i < numVolVertices * 3; i++)
	{
		vertexPos[i] = volume_vertices[i / 3][i % 3];
	}
	
	float* faceNromals = new float[numVolVertices];
	vector<Triangle> triangleList4BlockDetect = this->GetVolumeTriangleList();
	for (int i = 0; i < numVolVertices; i++)
	{
		faceNromals[i] = triangleList4BlockDetect[i / 3].faceNormal[i % 3];
	}
	
	float * transferCoeffsMat = new float[numEntries];
	float * transferMat = new float[sampleset.numSamples * sampleset.numSamples];
	float * tempMat = new float[sampleset.numSamples * numFunctions];
	//Create space for the SH coefficients in each vertex
	for (int i = 0; i < numVertices; ++i)
	{
		float dotNormal[3] = { normals[i].x,normals[i].y ,normals[i].z };
		float dotPos[3] = { vertices[i].x,vertices[i].y ,vertices[i].z };
		
		CLtool.GetSpecTransferMat(sampleset.numSamples, volume_vertices.size(), sampleDircts, dotNormal, dotPos, vertexPos, faceNromals, transferMat);
		CLtool.MatMultip(numFunctions, sampleset.numSamples, sampleset.numSamples, SH_BasicMat, transferMat, tempMat);
		CLtool.MatMultip(numFunctions, numFunctions, sampleset.numSamples, tempMat, SH_BasicMat_T, transferCoeffsMat);
		float specResult = 0;
		for (int j = 0; j < sampleset.numSamples * sampleset.numSamples; j++)
			specResult += transferMat[j];
		coeffsMatList.push_back(CoeffsMat(transferCoeffsMat));
		printf("Vertex %d done  res: %f\n", i,specResult);
	}
	delete[] tempMat;
	delete[] transferMat;
	delete[] transferCoeffsMat;
	delete[] faceNromals;
	delete[] vertexPos;
	delete[] sampleDircts;
	delete[] SH_BasicMat_T;
	delete[] SH_BasicMat;
}

void Scene::ExportAllTransMats(float * target)
{
	for (int i = 0; i < numVertices; i++)
	{
		for (int j = 0; j < 256; j++)
		{
			target[i * 256 + j] = this->coeffsMatList[i].Array[j];
		}
	}
}