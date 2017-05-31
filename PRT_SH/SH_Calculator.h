#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
class SH_Calculator
{
public:
	SH_Calculator();
	~SH_Calculator();
	//Evaluate an Associated Legendre Polynomial P(l, m) at x
	double SH_Calculator::P(int l, int m, double x);

	//Calculate the normalisation constant for an SH function
	double SH_Calculator::K(int l, int m);

	//Sample a spherical harmonic basis function Y(l, m) at a point on the unit sphere
	double SH_Calculator::SH(int l, int m, double theta, double phi);


	//Calculate n!
	int SH_Calculator::Factorial(int n);
};

