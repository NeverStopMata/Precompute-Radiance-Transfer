#include "Sample.h"



Sample::Sample() : shValues(NULL)
{
}


Sample::~Sample()
{
	if (shValues)
		delete[] shValues;
	shValues = NULL;
}
