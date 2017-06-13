#include "CoeffsMat.h"



CoeffsMat::CoeffsMat()
{

}
CoeffsMat::CoeffsMat(float* coeffMatArray)
{
	for (int i = 0; i < 256; i++)
	{
		Array[i] = coeffMatArray[i];
	}
}

CoeffsMat::~CoeffsMat()
{
}
