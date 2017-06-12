#pragma once
#include <vector>
#include "Sample.h"
#include "SH_Calculator.h"
using namespace std;
class SampleSet
{
public:
	vector<Sample>all;
	int numSamples;
	int numBands;
public:
	SampleSet();
	SampleSet(int sqrtNumSamples, int numBands);
	~SampleSet();
};

