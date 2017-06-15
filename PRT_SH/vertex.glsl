#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;
layout(location = 3) in vec4 coeffs1;
layout(location = 4) in vec4 coeffs2;
layout(location = 5) in vec4 coeffs3;
layout(location = 6) in vec4 coeffs4;
layout(location = 7) in vec4 tlcoeffs1;
layout(location = 8) in vec4 tlcoeffs2;
layout(location = 9) in vec4 tlcoeffs3;
layout(location = 10) in vec4 tlcoeffs4;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out float brightness;
// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform mat4 lightCoeffs;
uniform mat4 brdfCoeffs;
uniform vec3 EyePos_worldspace;



float[3] matrixMulptiVec3(bool transpose, float[9] matrix,float[3] orig_vec)
{
	float res[3];
	int i,j;
	for (i = 0; i<3; i++)
	{
		//Clear this entry of outVector
		res[i] = 0.0;

		//Loop through matrix row/column
		for (j = 0; j<3; j++)
		{
			if (transpose)
				res[i] += matrix[j*3 + i] * orig_vec[j];
			else
				res[i] += matrix[i*3 + j] * orig_vec[j];
		}
	}
	return res;
}
float[5] matrixMulptiVec5(bool transpose,float[25] matrix,float[5] orig_vec)
{
	float res[5];
	int i,j;
	for (i = 0; i<5; i++)
	{
		//Clear this entry of outVector
		res[i] = 0.0;

		//Loop through matrix row/column
		for (j = 0; j<5; j++)
		{
			if (transpose)
				res[i] += matrix[j*5 + i] * orig_vec[j];
			else
				res[i] += matrix[i*5 + j] * orig_vec[j];
		}
	}
	return res;
}

float[7] matrixMulptiVec7(bool transpose,float[49] matrix,float[7] orig_vec)
{
	float res[7];
	int i,j;
	for (i = 0; i<7; i++)
	{
		//Clear this entry of outVector
		res[i] = 0.0;

		//Loop through matrix row/column
		for (j = 0; j<7; j++)
		{
			if (transpose)
				res[i] += matrix[j*7 + i] * orig_vec[j];
			else
				res[i] += matrix[i*7 + j] * orig_vec[j];
		}
	}
	return res;
}


float[9] GetX90DegreeRotationMatrix3()
{
	float entries[9];
	entries[0] = 0.0;
	entries[1] = 1.0;
	entries[2] = 0.0;
	entries[3] = -1.0;
	entries[4] = 0.0;
	entries[5] = 0.0;
	entries[6] = 0.0;
	entries[7] = 0.0;
	entries[8] = 1.0;
	return entries;
}

float[25] GetX90DegreeRotationMatrix5()
{
	float entries[25];
	entries[0] = 0.0;
	entries[1] = 0.0;
	entries[2] = 0.0;
	entries[3] = 1.0;
	entries[4] = 0.0;
	entries[5] = 0.0;
	entries[6] = -1.0;
	entries[7] = 0.0;
	entries[8] = 0.0;
	entries[9] = 0.0;
	entries[10] = 0.0;
	entries[11] = 0.0;
	entries[12] = -0.5;
	entries[13] = 0.0;
	entries[14] = -sqrt(3.0) / 2;
	entries[15] = -1.0;
	entries[16] = 0.0;
	entries[17] = 0.0;
	entries[18] = 0.0;
	entries[19] = 0.0;
	entries[20] = 0.0;
	entries[21] = 0.0;
	entries[22] = -sqrt(3.0) / 2;
	entries[23] = 0.0;
	entries[24] = 0.5;
	return entries;
}

float[49] GetX90DegreeRotationMatrix7()
{
	float entries[49];
	for (int i = 0; i<49; ++i)
		entries[i] = 0.0;

	entries[3] = -sqrt(0.625);
	entries[5] = sqrt(0.375);
	entries[8] = -1.0;
	entries[17] = -sqrt(0.375);
	entries[19] = -sqrt(0.625);
	entries[21] = sqrt(0.625);
	entries[23] = sqrt(0.375);
	entries[32] = -0.25;
	entries[34] = -sqrt(15.0) / 4;
	entries[35] = -sqrt(0.375);
	entries[37] = sqrt(0.625);
	entries[46] = -sqrt(15.0) / 4;
	entries[48] = 0.25;
	return entries;
}

float[9] GetZRotationMatrix3(float angle)
{
	float entries[9];
	int currentEntry = 0;
	int i,j;
	//Loop through the rows and columns of the matrix
	for (i = 0; i<3; ++i)
	{
		for (j = 0; j<3; ++j, ++currentEntry)
		{
			//Initialise this entry to zero
			entries[currentEntry] = 0.0;

			if (i == 1)
			{
				if (j == 1)
					entries[currentEntry] = 1.0;
				continue;
			}


			//The angle used is k*angle where k=(size-1)/2-i
			if (i<1)
			{
				if (j == 0)
					entries[currentEntry] = cos(angle);

				if (j == 2)
					entries[currentEntry] = sin(angle);

				continue;
			}

			if (i>1)
			{
				if (j == 2)
					entries[currentEntry] = cos(angle);

				if (j == 0)
					entries[currentEntry] = -sin(angle);

				continue;
			}
		}
	}
	return entries;
}

float[25] GetZRotationMatrix5(float angle)
{
	int size = 5;
	float entries[25]; 
	//Convert angle to radians

	//Entry index
	int currentEntry = 0;
	int i,j;
	//Loop through the rows and columns of the matrix
	for (i = 0; i<size; ++i)
	{
		for (j = 0; j<size; ++j, ++currentEntry)
		{
			//Initialise this entry to zero
			entries[currentEntry] = 0.0;

			//For the central row (i=(size-1)/2), entry is 1 if j==i, else zero
			if (i == (size - 1) / 2)
			{
				if (j == i)
					entries[currentEntry] = 1.0;

				continue;
			}

			//For i<(size-1)/2, entry is cos if j==i or sin if j==size-i-1
			//The angle used is k*angle where k=(size-1)/2-i
			if (i<(size - 1) / 2)
			{
				int k = (size - 1) / 2 - i;

				if (j == i)
					entries[currentEntry] = cos(k*angle);

				if (j == size - i - 1)
					entries[currentEntry] = sin(k*angle);

				continue;
			}

			//For i>(size-1)/2, entry is cos if j==i or -sin if j==size-i-1
			//The angle used is k*angle where k=i-(size-1)/2
			if (i>(size - 1) / 2)
			{
				int k = i - (size - 1) / 2;

				if (j == i)
					entries[currentEntry] = cos(k*angle);

				if (j == size - i - 1)
					entries[currentEntry] = -sin(k*angle);

				continue;
			}
		}
	}

	return entries;
}

float[49] GetZRotationMatrix7(float angle)
{
	int size = 7;
	float entries[49]; 
	//Convert angle to radians

	//Entry index
	int currentEntry = 0;
	int i,j;
	//Loop through the rows and columns of the matrix
	for (i = 0; i<size; ++i)
	{
		for (j = 0; j<size; ++j, ++currentEntry)
		{
			//Initialise this entry to zero
			entries[currentEntry] = 0.0;

			//For the central row (i=(size-1)/2), entry is 1 if j==i, else zero
			if (i == (size - 1) / 2)
			{
				if (j == i)
					entries[currentEntry] = 1.0;

				continue;
			}

			//For i<(size-1)/2, entry is cos if j==i or sin if j==size-i-1
			//The angle used is k*angle where k=(size-1)/2-i
			if (i<(size - 1) / 2)
			{
				int k = (size - 1) / 2 - i;

				if (j == i)
					entries[currentEntry] = cos(k*angle);

				if (j == size - i - 1)
					entries[currentEntry] = sin(k*angle);
				continue;
			}

			//For i>(size-1)/2, entry is cos if j==i or -sin if j==size-i-1
			//The angle used is k*angle where k=i-(size-1)/2
			if (i>(size - 1) / 2)
			{
				int k = i - (size - 1) / 2;

				if (j == i)
					entries[currentEntry] = cos(k*angle);

				if (j == size - i - 1)
					entries[currentEntry] = -sin(k*angle);
				continue;
			}
		}
	}
	return entries;
}

float[16] getRotSpecCoeffs(vec4 component1, vec4 component2, vec4 component3, vec4 component4, float theta, float phi)
{
	float res[16];
	res[0] = component1[0];
	float order1[3];
	order1[0] = component1[1];
	order1[1] = component1[2];
	order1[2] = component1[3];
	float order2[5];
	order2[0] = component2[0];
	order2[1] = component2[1];
	order2[2] = component2[2];
	order2[3] = component2[3];
	order2[4] = component3[0];
	float order3[7];
	order3[0] = component3[1];
	order3[1] = component3[2];
	order3[2] = component3[3];
	order3[3] = component4[0];
	order3[4] = component4[1];
	order3[5] = component4[2];
	order3[6] = component4[3];
	float temp_result3[3];
	temp_result3 = matrixMulptiVec3(false, GetZRotationMatrix3(phi), matrixMulptiVec3(true, GetX90DegreeRotationMatrix3(), matrixMulptiVec3(false, GetZRotationMatrix3(theta), matrixMulptiVec3(false, GetX90DegreeRotationMatrix3(), order1))));
	res[1] = temp_result3[0];
	res[2] = temp_result3[1];
	res[3] = temp_result3[2];
	float temp_result5[5];
	temp_result5 = matrixMulptiVec5(false, GetZRotationMatrix5(phi), matrixMulptiVec5(true, GetX90DegreeRotationMatrix5(), matrixMulptiVec5(false, GetZRotationMatrix5(theta), matrixMulptiVec5(false, GetX90DegreeRotationMatrix5(), order2))));
	res[4] = temp_result5[0];
	res[5] = temp_result5[1];
	res[6] = temp_result5[2];
	res[7] = temp_result5[3];
	res[8] = temp_result5[4];
	float temp_result7[7];
	temp_result7 = matrixMulptiVec7(false, GetZRotationMatrix7(phi), matrixMulptiVec7(true, GetX90DegreeRotationMatrix7(), matrixMulptiVec7(false, GetZRotationMatrix7(theta), matrixMulptiVec7(false, GetX90DegreeRotationMatrix7(), order3))));
	res[9] = temp_result7[0];
	res[10] = temp_result7[1];
	res[11] = temp_result7[2];
	res[12] = temp_result7[3];
	res[13] = temp_result7[4];
	res[14] = temp_result7[5];
	res[15] = temp_result7[6];
	return res;
}


void main(){

	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
	vec3 PosWorldSpace = (M * vec4(vertexPosition_modelspace, 1)).xyz;
	vec3 V_dir = normalize(EyePos_worldspace - PosWorldSpace);
	float theta = acos(V_dir.z);
	float phi;
	if(V_dir.y / sin(theta) > 0.99f || V_dir.y / sin(theta) < -0.99f)
		phi = V_dir.y > 0 ? 1.570963f : -1.570963f;
	else
		phi = V_dir.x > 0 ? asin(V_dir.y / sin(theta)) : (3.1415926f - asin(V_dir.y / sin(theta)));


	float brdfCoe1[3];
	brdfCoe1[0] = brdfCoeffs[0].y;
	brdfCoe1[1] = brdfCoeffs[0].z;
	brdfCoe1[2] = brdfCoeffs[0].w;

	float brdfCoe2[5];
	brdfCoe2[0] = brdfCoeffs[1].x;
	brdfCoe2[1] = brdfCoeffs[1].y;
	brdfCoe2[2] = brdfCoeffs[1].z;
	brdfCoe2[3] = brdfCoeffs[1].w;
	brdfCoe2[4] = brdfCoeffs[2].x;
	
	float brdfCoe3[7];
	brdfCoe3[0] = brdfCoeffs[2].y;
	brdfCoe3[1] = brdfCoeffs[2].z;
	brdfCoe3[2] = brdfCoeffs[2].w;
	brdfCoe3[3] = brdfCoeffs[3].x;
	brdfCoe3[4] = brdfCoeffs[3].y;
	brdfCoe3[5] = brdfCoeffs[3].z;
	brdfCoe3[6] = brdfCoeffs[3].w;

	float resCoe1[3] = matrixMulptiVec3(false, GetZRotationMatrix3(phi), matrixMulptiVec3(true, GetX90DegreeRotationMatrix3(), matrixMulptiVec3(false, GetZRotationMatrix3(theta), matrixMulptiVec3(false, GetX90DegreeRotationMatrix3(), brdfCoe1))));
	float resCoe2[5] = matrixMulptiVec5(false, GetZRotationMatrix5(phi), matrixMulptiVec5(true, GetX90DegreeRotationMatrix5(), matrixMulptiVec5(false, GetZRotationMatrix5(theta), matrixMulptiVec5(false, GetX90DegreeRotationMatrix5(), brdfCoe2))));
	float resCoe3[7] = matrixMulptiVec7(false, GetZRotationMatrix7(phi), matrixMulptiVec7(true, GetX90DegreeRotationMatrix7(), matrixMulptiVec7(false, GetZRotationMatrix7(theta), matrixMulptiVec7(false, GetX90DegreeRotationMatrix7(), brdfCoe3))));
	mat4 rotatedBrdfCoes;
	rotatedBrdfCoes[0] = vec4(brdfCoeffs[0].x, resCoe1[0], resCoe1[1], resCoe1[2]);
	rotatedBrdfCoes[1] = vec4(resCoe2[0],      resCoe2[1], resCoe2[2], resCoe2[3]);
	rotatedBrdfCoes[2] = vec4(resCoe2[4],      resCoe3[0], resCoe3[1], resCoe3[2]);
	rotatedBrdfCoes[3] = vec4(resCoe3[3],      resCoe3[4], resCoe3[5], resCoe3[6]);
	// UV of the vertex. No special space for this one.
	UV = vertexUV;
	float diffuseBritness = 0.0f;
	diffuseBritness += dot(lightCoeffs[0], coeffs1);
	diffuseBritness += dot(lightCoeffs[1], coeffs2);
	diffuseBritness += dot(lightCoeffs[2], coeffs3);
	diffuseBritness += dot(lightCoeffs[3], coeffs4);
	float SpecuBritness = 0.0f;
	SpecuBritness += dot(rotatedBrdfCoes[0], tlcoeffs1);
	SpecuBritness += dot(rotatedBrdfCoes[1], tlcoeffs2);
	SpecuBritness += dot(rotatedBrdfCoes[2], tlcoeffs3);
	SpecuBritness += dot(rotatedBrdfCoes[3], tlcoeffs4);
	float f0 = 0.6f;
	vec3 N = normalize((M * vec4(vertexNormal_modelspace, 0)).xyz);
	float fresnel = f0 + (1.0 - f0) * pow(1.0f - dot(V_dir, N), 5.0f);
	brightness = mix(diffuseBritness, SpecuBritness, fresnel);
}

