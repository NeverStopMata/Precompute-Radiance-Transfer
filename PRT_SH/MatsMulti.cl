#define EPSILON 0.01
void AtomicAddFloat(volatile global float *source, const float operand)
{
	union {
		unsigned int intVal;
		float floatVal;
	}newVal;
	union {
		unsigned int intVal;
		float floatVal;
	}prevVal;
	do {
		prevVal.floatVal = *source;
		newVal.floatVal = prevVal.floatVal + operand;
	} while (atomic_cmpxchg((volatile global unsigned int *)source,
		prevVal.intVal, newVal.intVal) != prevVal.intVal);
}

bool IsBlocked(float3 S, float3 dir, int numVertices, global float* vertPos, global float* faceNormals)
{
	float3 A, B, C;
	float distance;
	for (int i = 0; i < numVertices / 3; i++)
	{
		A = (float3)(vertPos[i * 9], vertPos[i * 9 + 1], vertPos[i * 9 + 2]);
		float3 currentTriNormal = (float3)(faceNormals[i * 3], faceNormals[i * 3 + 1], faceNormals[i * 3 + 2]);
		distance = dot(A - S, currentTriNormal) / dot(currentTriNormal, dir);
		if (distance < 0.005)
		{
			continue;
		}
		float3 P = S + dir * distance;
		B = (float3)(vertPos[i * 9 + 3], vertPos[i * 9 + 4], vertPos[i * 9 + 5]);
		C = (float3)(vertPos[i * 9 + 6], vertPos[i * 9 + 7], vertPos[i * 9 + 8]);
		float3 AB = B - A;
		float3 BP = P - B;
		if (dot(currentTriNormal, cross(AB, BP)) <= 0)
			continue;
		float3 BC = C - B;
		float3 CP = P - C;
		if (dot(currentTriNormal, cross(BC, CP)) <= 0)
			continue;
		float3 CA = A - C;
		float3 AP = P - A;
		if (dot(currentTriNormal, cross(CA, AP)) <= 0)
			continue;
		return true;
	}

	return false;
}

float8 IsBlockedExtend(float3 S, float3 dir, int numVertices, global float* vertPos, global float* faceNormals)
{
	float3 A, B, C;
	float distance;
	int numTri = numVertices / 3;
	for (int i = 0; i < numTri; i++)
	{
		A = (float3)(vertPos[i * 9], vertPos[i * 9 + 1], vertPos[i * 9 + 2]);
		float3 currentTriNormal = (float3)(faceNormals[i * 3], faceNormals[i * 3 + 1], faceNormals[i * 3 + 2]);
		distance = dot(A - S, currentTriNormal) / dot(currentTriNormal, dir);
		if (distance < 0.005)
		{
			continue;
		}
		float3 P = S + dir * distance;
		B = (float3)(vertPos[i * 9 + 3], vertPos[i * 9 + 4], vertPos[i * 9 + 5]);
		C = (float3)(vertPos[i * 9 + 6], vertPos[i * 9 + 7], vertPos[i * 9 + 8]);
		float3 AB = B - A;
		float3 BP = P - B;
		if (dot(currentTriNormal, cross(AB, BP)) <= 0)
			continue;
		float3 BC = C - B;
		float3 CP = P - C;
		if (dot(currentTriNormal, cross(BC, CP)) <= 0)
			continue;
		float3 CA = A - C;
		float3 AP = P - A;
		if (dot(currentTriNormal, cross(CA, AP)) <= 0)
			continue;
		return (float8)(P, 0.0f, currentTriNormal, 1.0f);
	}

	return (float8)(0.0f);
}

inline float3 reflec(float3 in, float3 normal)
{
	float length = 2.0f * dot(in, normal);
	return (normal * length - in);
}

void kernel MatsMulti(global const int* IntArg, global const float* A, global const float* B, global float* C)
{
	const int row = get_local_id(0);
	const int col = get_local_id(1);
	int M = IntArg[0];
	int N = IntArg[1];
	int K = IntArg[2];
	const int globalRow = M * get_group_id(0) + row;
	const int globalCol = M * get_group_id(1) + col;
	local float Asub[16][16];
	local float Bsub[16][16];
	float acc = 0.0f;
	const int numTiles = K / M;
	for (int t = 0; t < numTiles; t++) {
		const int tiledRow = M*t + row;
		const int tiledCol = M*t + col;
		Asub[col][row] = A[tiledCol*M + globalRow];
		Bsub[col][row] = B[globalCol*K + tiledRow];
		barrier(CLK_LOCAL_MEM_FENCE);
		for (int k = 0; k < M; k++) {
			acc += Asub[k][row] * Bsub[col][k];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}
	C[globalCol*M + globalRow] = acc;
}

void kernel InitSceneMat(global const int* IntArg, global const float* sampleDircts, global const float* dotNormals,
	global const float* dotPos, global const float* vertPos, global const float* faceNormals, global float* res)
{
	const int dotNum = get_global_id(1);
	const int sampleNum = get_global_id(0);
	int numSample = IntArg[0];
	int numDot = IntArg[1];
	int numVertices = IntArg[2];
	const int entryNum = dotNum*numSample + sampleNum;

	float3 normal = (float3)(dotNormals[dotNum * 3], dotNormals[dotNum * 3 + 1], dotNormals[dotNum * 3 + 2]);
	float3 sampleDir = (float3)(sampleDircts[sampleNum * 3], sampleDircts[sampleNum * 3 + 1], sampleDircts[sampleNum * 3 + 2]);
	float dot_value = dot(normal, sampleDir);
	float8 blockInfo;
	float3 blockPos;
	float3 blockNormal;
	bool isBlocked;
	float3 S;

	if (dot_value < 0)
	{
		res[entryNum] = 0;
		return;
	}
	else
	{
		S = (float3)(dotPos[dotNum * 3], dotPos[dotNum * 3 + 1], dotPos[dotNum * 3 + 2]);
		isBlocked = IsBlocked(S, sampleDir, numVertices, vertPos, faceNormals);
		res[entryNum] = isBlocked ? 0.0f : (dot_value*4.0f*M_PI / numSample);
	}
}


void kernel GetSpecTransferMat_IR(global const int* IntArg, global const float* sampleDircts, global const float* dotNormal,
	global float* dotPos, global float* vertPos, global float* faceNormals, global float* res)
{
	int numSample = IntArg[0];
	int numVertices = IntArg[1];
	const int row = get_global_id(0);
	const int col = get_global_id(1);
	const int entry = col * numSample + row;
	float3 sampleIn_Dir = (float3)(sampleDircts[row * 3], sampleDircts[row * 3 + 1], sampleDircts[row * 3 + 2]);
	float3 sampleOutDir = (float3)(sampleDircts[col * 3], sampleDircts[col * 3 + 1], sampleDircts[col * 3 + 2]);
	float3 normal = (float3)(dotNormal[0], dotNormal[1], dotNormal[2]);
	float dot_value = dot(normal, sampleOutDir);
	float acc = 0.0f;
	if (dot_value <= 0)
	{
		return;
	}
	float3 S = (float3)(dotPos[0], dotPos[1], dotPos[2]);
	float3 reflcDir = reflec(sampleOutDir, normal);
	float8 blockInfo;
	bool isBlocked;
	isBlocked = ((blockInfo = IsBlockedExtend(S, reflcDir, numVertices, vertPos, faceNormals)).hi.w == 1.0f);
	if (!isBlocked)
	{
		acc += (dot(reflcDir, sampleIn_Dir) >= 0.9995116f)?dot_value:0.0f;
	}
	else
	{
		return;
		float3 blockNormal = blockInfo.hi.xyz;
		float3 interReflcDir = reflec(-reflcDir, blockNormal);
		acc += (dot(interReflcDir, sampleIn_Dir) >= 0.9995116f) ? (dot_value*dot(blockNormal, -reflcDir)) : 0.0f;
	}
	res[entry] = acc;
}

void kernel GetSpecTransferMat(global const int* IntArg, global const float* sampleDircts, global const float* dotNormal,
	global float* dotPos, global float* vertPos, global float* faceNormals, global float* res)
{
	int numSample = IntArg[0];
	int numVertices = IntArg[1];
	const int row = get_global_id(0);
	const int col = get_global_id(1);
	int entry = col * numSample + row;
	res[entry] = 0.0f;
	float3 sampleIn_Dir = (float3)(sampleDircts[row * 3], sampleDircts[row * 3 + 1], sampleDircts[row * 3 + 2]);
	float3 normal = (float3)(dotNormal[0], dotNormal[1], dotNormal[2]);
	float dot_value = dot(normal, sampleIn_Dir);
	if (dot_value <= 0)
	{
		return;
	}
	float3 reflcDir = reflec(sampleIn_Dir, normal);
	float3 sampleOutDir = (float3)(sampleDircts[col * 3], sampleDircts[col * 3 + 1], sampleDircts[col * 3 + 2]);
	if (dot(reflcDir, sampleOutDir) < 0.9995116f)
		return;
	float3 S = (float3)(dotPos[0], dotPos[1], dotPos[2]);
	if (IsBlocked(S, sampleIn_Dir, numVertices, vertPos, faceNormals))
		return;
	res[entry] = dot_value;
}



void kernel GetSpecBrightness(global const int* IntArg, global const float* lightCoeffs, global const float* brdfCoeffs,
	global const float* transferMats, global float* res)
{
	const int verNum = get_global_id(0);

	float temp[16];
	for (int i = 0; i < 16; i++)
	{
		temp[i] = 0.0f;
		for (int j = 0; j < 16; j++)
		{
			temp[i] += lightCoeffs[j] * transferMats[256 * verNum + 16 * i + j];
		}
	}
	float brightness = 0.0f;
	for (int i = 0; i < 16; i++)
		brightness += temp[i] * brdfCoeffs[i];

	res[verNum] = brightness;
}

void kernel GetTransLightCoes(global const int* IntArg, global const float* lightCoeffs,
	global const float* transferMats, global float* res)
{
	const int bitNum = get_global_id(0);
	const int verNum = get_global_id(1);
	float acc = 0.0f;
	for (int i = 0; i < 16; i++)
	{
		acc += lightCoeffs[i] * transferMats[256 * verNum + 16 * bitNum + i];
	}
	res[verNum * 16 + bitNum] = acc;
}



void kernel GetDifInterRefMat(global const int* IntArg, global const float* sampleDircts, global const float* dotNormal,
	global float* dotPos, global float* vertPos, global float* faceNormals, global float* res)
{
	int numSample = IntArg[1];
	int numVertices = IntArg[2];
	const int row = get_global_id(0);
	const int col = get_global_id(1);
	const int entry = col * numSample + row;
	float3 inDir1 = (float3)(sampleDircts[row * 3], sampleDircts[row * 3 + 1], sampleDircts[row * 3 + 2]);
	float3 normal1 = (float3)(dotNormal[0], dotNormal[1], dotNormal[2]);
	float dot_value1 = dot(normal1, inDir1);
	if (dot_value1 <= 0)
	{
		res[entry] = 0.0f;
	}
	else
	{
		float3 S = (float3)(dotPos[0], dotPos[1], dotPos[2]);
		float8 blockedInfo = IsBlockedExtend(S, inDir1, numVertices, vertPos, faceNormals);
		if (blockedInfo.hi.w == 0.0f)
		{
			res[entry] = 0.0f;
		}
		else
		{
			float3 inDir2 = (float3)(sampleDircts[col * 3], sampleDircts[col * 3 + 1], sampleDircts[col * 3 + 2]);
			float3 normal2 = blockedInfo.hi.xyz;
			float dot_value2 = dot(normal2, inDir2);
			if (dot_value2 <= 0.0f)
			{
				res[entry] = 0.0f;
			}
			else if(IsBlocked(blockedInfo.lo.xyz, inDir2, numVertices, vertPos, faceNormals))
			{
				res[entry] = 0.0f;
			}
			else
			{
				res[entry] = dot_value1 * dot_value2 * 100.0f / numSample;
			}
		}
	}
}



void kernel MatToVec(global const int* IntArg, global const float* SourceMat, global float* res)
{
	int numSample = IntArg[1];
	const int col = get_global_id(0);
	int beginPos = col * numSample;
	float acc = 0.0f;
	for (int i = 0; i < numSample; i++)
	{
		acc += SourceMat[beginPos + i];
	}
	res[col] = acc * M_PI * 4.0f /(numSample);
}



void kernel VecToCoeff(global const int* IntArg, global const float* SH_Mat_T, global const float* Vec, global float* res)
{
	int numFunc = IntArg[0];
	int numSample = IntArg[1];
	const int col = get_global_id(0);
	float acc = 0.0f;
	int beginPos = col * numSample;
	for (int i = 0; i < numSample; i++)
	{
		acc += Vec[i] * SH_Mat_T[beginPos + i];
	}
	res[col] = acc;
}

