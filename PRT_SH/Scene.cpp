#include "Scene.h"

#include "common/objloader.hpp"
#include "common/vboindexer.hpp"
#include <iostream>
#include <fstream>
using namespace std;
using namespace glm;
Scene::Scene(const char * filePath,SampleSet Ss, bool hasPrecomputed)
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
	SubFacesGenerate(stack_triangles);
	volume_vertices = temp_vertices;
	numVolVertices = volume_vertices.size();
	numVertices = vertices.size();
	
	
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
	numIndices = indexed_vertices.size();
	if (hasPrecomputed)
	{
		ImportCoeffsFromFile();
		
	}
	else
	{
		GenerateDirectCoeffs_CL(Ss);
		GenerateInterRCoeffs_CL(Ss);
		GenerateTransferMatrix_CL(Ss);
		ExportCoeffsToFile();
	}
}

Scene::~Scene()
{
	volume_vertices.clear();
	vertices.clear();
	uvs.clear();
	normals.clear();
	indices.clear();
	indexed_vertices.clear();
	indexed_uvs.clear();
	indexed_normals.clear();
	indexed_coeffsVecList.clear();
	indexed_coeffsMatList.clear();
}

void Scene::SubFacesGenerate(stack<Triangle> stack_triangles)
{	
	//cout << "test" << temp_vertices.size() << "/" << temp_uvs.size() << "/" << temp_normals.size() << endl;
	while (!stack_triangles.empty())
	{
		//cout << stack_triangles.size() << endl;
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


	vector<Triangle> triangleList4BlockDetect = this->GetVolumeTriangleList();
	for (int i = 0; i < numIndices; ++i)
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
			float dotResult = dot(sampleset.all[j].direction, indexed_normals[i]);
			
			if (dotResult > 0.0f)
			{
				Ray currentRay(this->indexed_vertices[i], sampleset.all[j].direction, indexed_normals[i]);
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
		this->indexed_coeffsVecList.push_back(Coeffs);
		//cout <<"current vertex: NO. "<< i <<"  "<<"blocked rays number:"<<blockNum<< endl;
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
	float* dotNormals = new float[numIndices * 3];
	for (int i = 0; i < numIndices * 3; i++)
	{
		dotNormals[i] = indexed_normals[i / 3][i % 3];
	}
	float* dotPos = new float[numIndices * 3];
	for (int i = 0; i < numIndices * 3; i++)
	{
		dotPos[i] = indexed_vertices[i / 3][i % 3];
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
	numDots_round = (numIndices / 16 + 1)*16;
	float* sceneMatrix = new float[sampleset.numSamples * numDots_round];
	for (int i = sampleset.numSamples * numIndices; i < sampleset.numSamples * numDots_round; i++)
		sceneMatrix[i] = 0.0f;
	CLtool.GetSceneMatrix(sampleset.numSamples, numIndices, numVolVertices, sampleDircts, dotNormals, dotPos, vertexPos, faceNromals, sceneMatrix);
	int numSH_Mat = sampleset.numSamples * sampleset.numBands * sampleset.numBands;
	float * SH_BasicMat = new float[numSH_Mat];
	for (int i = 0; i < numSH_Mat; i++)
	{
		SH_BasicMat[i] = sampleset.all[i/16].shValues.Array[i%16];
	}
	float * coeffMat = new float[sampleset.numBands * sampleset.numBands * numDots_round];
	CLtool.MatMultip(16, numDots_round, sampleset.numSamples, SH_BasicMat, sceneMatrix, coeffMat);
	for (int i = 0; i < numIndices; i++)
	{
		CoeffsVector16 tempCoeffs;
		for (int j = 0; j < 16; j++)
			tempCoeffs.Array[j] = coeffMat[i*sampleset.numBands * sampleset.numBands + j];
		indexed_coeffsVecList.push_back(tempCoeffs);
		cout << i << endl;
	}

	delete[] sampleDircts;
	delete[] dotNormals;
	delete[] dotPos;
	delete[] vertexPos;
	delete[] faceNromals;
	delete[] sceneMatrix;
	delete[] SH_BasicMat;
}


void Scene::GenerateInterRCoeffs_CL(SampleSet sampleset)
{
	OpenCL_Math CLtool;
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

	int numFunc = sampleset.numBands * sampleset.numBands;
	int numSH_Mat = sampleset.numSamples * sampleset.numBands * sampleset.numBands;
	float * SH_BasicMat_T = new float[numSH_Mat];
	for (int i = 0; i < numSH_Mat; i++)
	{
		SH_BasicMat_T[i] = sampleset.all[i % sampleset.numSamples].shValues.Array[i / sampleset.numSamples] * 4.0f * M_PI / sampleset.numSamples;
	}

	float * InterReflecCoeff = new float[numFunc];
	for (int i = 0; i < numIndices; i++)
	{
		float dotNormal[3] = { indexed_normals[i].x,indexed_normals[i].y ,indexed_normals[i].z };
		float dotPos[3] = { indexed_vertices[i].x,indexed_vertices[i].y ,indexed_vertices[i].z };
		
		CLtool.GetDifInterRefCoeff(sampleset.numSamples, numVolVertices, numFunc, SH_BasicMat_T, sampleDircts, dotNormal, dotPos, vertexPos, faceNromals, InterReflecCoeff);
		CoeffsVector16 tempCoeffs;
		for (int i = 0; i < 16; i++)
			tempCoeffs.Array[i] = InterReflecCoeff[i];
		indexed_IRcoesVecList.push_back(tempCoeffs);
		printf("done V%d's interreflection Coeff\n", i);
		
	}
	delete[] InterReflecCoeff;
	delete[] sampleDircts;
	delete[] vertexPos;
	delete[] faceNromals;
	delete[] SH_BasicMat_T;
	//sampleDircts = NULL;
	//vertexPos = NULL;
	//faceNromals = NULL;
	//SH_BasicMat_T = NULL;
	//InterReflecCoeff = NULL;
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
		SH_BasicMat_T[i] = sampleset.all[i % sampleset.numSamples].shValues.Array[i / sampleset.numSamples] * 4.0f * M_PI / sampleset.numSamples;
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
	for (int i = 0; i < numIndices; ++i)
	{
		float dotNormal[3] = { indexed_normals[i].x,indexed_normals[i].y ,indexed_normals[i].z };
		float dotPos[3] = { indexed_vertices[i].x,indexed_vertices[i].y ,indexed_vertices[i].z };
		
		CLtool.GetSpecTransferMat(sampleset.numSamples, volume_vertices.size(), sampleDircts, dotNormal, dotPos, vertexPos, faceNromals, transferMat);
		CLtool.MatMultip(numFunctions, sampleset.numSamples, sampleset.numSamples, SH_BasicMat, transferMat, tempMat);
		CLtool.MatMultip(numFunctions, numFunctions, sampleset.numSamples, tempMat, SH_BasicMat_T, transferCoeffsMat);

		indexed_coeffsMatList.push_back(CoeffsMat(transferCoeffsMat));
		printf("Vertex %d done  res: \n", i);
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
	for (int i = 0; i < numIndices; i++)
	{
		for (int j = 0; j < 256; j++)
		{
			target[i * 256 + j] = this->indexed_coeffsMatList[i].Array[j];
		}
	}
}

void Scene::ExportCoeffsToFile()
{
	ofstream outFile("Coeff.data",ios::binary);
	if (!outFile)
	{
		cout << "open file failed!" << endl;
	}
	outFile.clear();
	for (int i = 0; i < numIndices; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			float temp = indexed_coeffsVecList[i].Array[j];
			outFile.write((char*)&temp, sizeof(temp));
		}
		for (int j = 0; j < 16; j++)
		{
			float temp = indexed_IRcoesVecList[i].Array[j];
			outFile.write((char*)&temp, sizeof(temp));
		}
		for (int j = 0; j < 16; j++)
		{
			for (int k = 0; k < 16; k++)
			{
				float temp = indexed_coeffsMatList[i].Array[j * 8 + k];
				outFile.write((char*)&temp, sizeof(temp));
			}
		}
	}
	outFile.close();
}

void Scene::ImportCoeffsFromFile()
{
	ifstream inFile("Coeff.data", ios::binary);

	for (int i = 0; i < numIndices; i++)
	{
		CoeffsVector16 DirectDiffuCoeffs;
		CoeffsVector16 InterRDiffuCoeffs;
		CoeffsMat TransLightMat;
		inFile.read((char*)&(DirectDiffuCoeffs.Array[0]), sizeof(float)*16);
		inFile.read((char*)&(InterRDiffuCoeffs.Array[0]), sizeof(float) * 16);
		inFile.read((char*)&(TransLightMat.Array[0]), sizeof(float) * 256);

		indexed_coeffsVecList.push_back(DirectDiffuCoeffs);
		indexed_IRcoesVecList.push_back(InterRDiffuCoeffs);
		indexed_coeffsMatList.push_back(TransLightMat);
	}
	inFile.close();
}