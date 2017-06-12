#include "CoeffsMat.h"



CoeffsMat::CoeffsMat()
{

}
CoeffsMat::CoeffsMat(float* coeffMatArray)
{
	for (int i = 0; i < 16; i++)
	{
		Array[i] = coeffMatArray[i];
	}
}

CoeffsMat::~CoeffsMat()
{
}
