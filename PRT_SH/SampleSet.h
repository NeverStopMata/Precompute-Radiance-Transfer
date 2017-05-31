#pragma once
#include "Sample.h"
#include "SH_Calculator.h"
class SampleSet
{
public:
	Sample * all;
	int numSamples;
	int numBands;
public:
	SampleSet();
	SampleSet(int sqrtNumSamples, int numBands);
	~SampleSet();
};

