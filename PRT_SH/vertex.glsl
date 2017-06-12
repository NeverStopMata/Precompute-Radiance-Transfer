#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;
layout(location = 3) in vec4 coeffs1;
layout(location = 4) in vec4 coeffs2;
layout(location = 5) in vec4 coeffs3;
layout(location = 6) in vec4 coeffs4;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out float brightness;
// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform mat4 lightCoeffs;
uniform vec3 EyePos_worldspace;



//float[3] matrixMulptiVec3(bool transpose, float[9] matrix,float[3] orig_vec)
//{
//	float res[3];
//	int i,j;
//	for (i = 0; i<3; i++)
//	{
//		//Clear this entry of outVector
//		res[i] = 0.0;
//
//		//Loop through matrix row/column
//		for (j = 0; j<3; j++)
//		{
//			if (transpose)
//				res[i] += matrix[j*3 + i] * orig_vec[j];
//			else
//				res[i] += matrix[i*3 + j] * orig_vec[j];
//		}
//	}
//	return res;
//}
//float[5] matrixMulptiVec5(bool transpose,float[25] matrix,float[5] orig_vec)
//{
//	float res[5];
//	int i,j;
//	for (i = 0; i<5; i++)
//	{
//		//Clear this entry of outVector
//		res[i] = 0.0;
//
//		//Loop through matrix row/column
//		for (j = 0; j<5; j++)
//		{
//			if (transpose)
//				res[i] += matrix[j*5 + i] * orig_vec[j];
//			else
//				res[i] += matrix[i*5 + j] * orig_vec[j];
//		}
//	}
//	return res;
//}
//
//float[7] matrixMulptiVec7(bool transpose,float[49] matrix,float[7] orig_vec)
//{
//	float res[7];
//	int i,j;
//	for (i = 0; i<7; i++)
//	{
//		//Clear this entry of outVector
//		res[i] = 0.0;
//
//		//Loop through matrix row/column
//		for (j = 0; j<7; j++)
//		{
//			if (transpose)
//				res[i] += matrix[j*7 + i] * orig_vec[j];
//			else
//				res[i] += matrix[i*7 + j] * orig_vec[j];
//		}
//	}
//	return res;
//}
//
//
//float[9] GetX90DegreeRotationMatrix3()
//{
//	float entries[9];
//	entries[0] = 0.0;
//	entries[1] = 1.0;
//	entries[2] = 0.0;
//	entries[3] = -1.0;
//	entries[4] = 0.0;
//	entries[5] = 0.0;
//	entries[6] = 0.0;
//	entries[7] = 0.0;
//	entries[8] = 1.0;
//	return entries;
//}
//
//float[25] GetX90DegreeRotationMatrix5()
//{
//	float entries[25];
//	entries[0] = 0.0;
//	entries[1] = 0.0;
//	entries[2] = 0.0;
//	entries[3] = 1.0;
//	entries[4] = 0.0;
//	entries[5] = 0.0;
//	entries[6] = -1.0;
//	entries[7] = 0.0;
//	entries[8] = 0.0;
//	entries[9] = 0.0;
//	entries[10] = 0.0;
//	entries[11] = 0.0;
//	entries[12] = -0.5;
//	entries[13] = 0.0;
//	entries[14] = -sqrt(3.0) / 2;
//	entries[15] = -1.0;
//	entries[16] = 0.0;
//	entries[17] = 0.0;
//	entries[18] = 0.0;
//	entries[19] = 0.0;
//	entries[20] = 0.0;
//	entries[21] = 0.0;
//	entries[22] = -sqrt(3.0) / 2;
//	entries[23] = 0.0;
//	entries[24] = 0.5;
//	return entries;
//}
//
//float[49] GetX90DegreeRotationMatrix7()
//{
//	float entries[49];
//	for (int i = 0; i<49; ++i)
//		entries[i] = 0.0;
//
//	entries[3] = -sqrt(0.625);
//	entries[5] = sqrt(0.375);
//	entries[8] = -1.0;
//	entries[17] = -sqrt(0.375);
//	entries[19] = -sqrt(0.625);
//	entries[21] = sqrt(0.625);
//	entries[23] = sqrt(0.375);
//	entries[32] = -0.25;
//	entries[34] = -sqrt(15.0) / 4;
//	entries[35] = -sqrt(0.375);
//	entries[37] = sqrt(0.625);
//	entries[46] = -sqrt(15.0) / 4;
//	entries[48] = 0.25;
//	return entries;
//}
//
//float[9] GetZRotationMatrix3(float angle)
//{
//	float entries[9];
//	int currentEntry = 0;
//	int i,j;
//	//Loop through the rows and columns of the matrix
//	for (i = 0; i<3; ++i)
//	{
//		for (j = 0; j<3; ++j, ++currentEntry)
//		{
//			//Initialise this entry to zero
//			entries[currentEntry] = 0.0;
//
//			if (i == 1)
//			{
//				if (j == 1)
//					entries[currentEntry] = 1.0;
//				continue;
//			}
//
//
//			//The angle used is k*angle where k=(size-1)/2-i
//			if (i<1)
//			{
//				if (j == 0)
//					entries[currentEntry] = cos(angle);
//
//				if (j == 2)
//					entries[currentEntry] = sin(angle);
//
//				continue;
//			}
//
//			if (i>1)
//			{
//				if (j == 2)
//					entries[currentEntry] = cos(angle);
//
//				if (j == 0)
//					entries[currentEntry] = -sin(angle);
//
//				continue;
//			}
//		}
//	}
//	return entries;
//}
//
//float[25] GetZRotationMatrix5(float angle)
//{
//	int size = 5;
//	float entries[25]; 
//	//Convert angle to radians
//
//	//Entry index
//	int currentEntry = 0;
//	int i,j;
//	//Loop through the rows and columns of the matrix
//	for (i = 0; i<size; ++i)
//	{
//		for (j = 0; j<size; ++j, ++currentEntry)
//		{
//			//Initialise this entry to zero
//			entries[currentEntry] = 0.0;
//
//			//For the central row (i=(size-1)/2), entry is 1 if j==i, else zero
//			if (i == (size - 1) / 2)
//			{
//				if (j == i)
//					entries[currentEntry] = 1.0;
//
//				continue;
//			}
//
//			//For i<(size-1)/2, entry is cos if j==i or sin if j==size-i-1
//			//The angle used is k*angle where k=(size-1)/2-i
//			if (i<(size - 1) / 2)
//			{
//				int k = (size - 1) / 2 - i;
//
//				if (j == i)
//					entries[currentEntry] = cos(k*angle);
//
//				if (j == size - i - 1)
//					entries[currentEntry] = sin(k*angle);
//
//				continue;
//			}
//
//			//For i>(size-1)/2, entry is cos if j==i or -sin if j==size-i-1
//			//The angle used is k*angle where k=i-(size-1)/2
//			if (i>(size - 1) / 2)
//			{
//				int k = i - (size - 1) / 2;
//
//				if (j == i)
//					entries[currentEntry] = cos(k*angle);
//
//				if (j == size - i - 1)
//					entries[currentEntry] = -sin(k*angle);
//
//				continue;
//			}
//		}
//	}
//
//	return entries;
//}
//
//float[49] GetZRotationMatrix7(float angle)
//{
//	int size = 7;
//	float entries[49]; 
//	//Convert angle to radians
//
//	//Entry index
//	int currentEntry = 0;
//	int i,j;
//	//Loop through the rows and columns of the matrix
//	for (i = 0; i<size; ++i)
//	{
//		for (j = 0; j<size; ++j, ++currentEntry)
//		{
//			//Initialise this entry to zero
//			entries[currentEntry] = 0.0;
//
//			//For the central row (i=(size-1)/2), entry is 1 if j==i, else zero
//			if (i == (size - 1) / 2)
//			{
//				if (j == i)
//					entries[currentEntry] = 1.0;
//
//				continue;
//			}
//
//			//For i<(size-1)/2, entry is cos if j==i or sin if j==size-i-1
//			//The angle used is k*angle where k=(size-1)/2-i
//			if (i<(size - 1) / 2)
//			{
//				int k = (size - 1) / 2 - i;
//
//				if (j == i)
//					entries[currentEntry] = cos(k*angle);
//
//				if (j == size - i - 1)
//					entries[currentEntry] = sin(k*angle);
//				continue;
//			}
//
//			//For i>(size-1)/2, entry is cos if j==i or -sin if j==size-i-1
//			//The angle used is k*angle where k=i-(size-1)/2
//			if (i>(size - 1) / 2)
//			{
//				int k = i - (size - 1) / 2;
//
//				if (j == i)
//					entries[currentEntry] = cos(k*angle);
//
//				if (j == size - i - 1)
//					entries[currentEntry] = -sin(k*angle);
//				continue;
//			}
//		}
//	}
//	return entries;
//}


void main(){

	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);

	// UV of the vertex. No special space for this one.
	UV = vertexUV;

	brightness = 0.0f;
	brightness += dot(lightCoeffs[0], coeffs1);
	brightness += dot(lightCoeffs[1], coeffs2);
	brightness += dot(lightCoeffs[2], coeffs3);
	brightness += dot(lightCoeffs[3], coeffs4);

}

