#include "math.hpp"

#include <string>
using std::string;

#include <cmath>

long math_pow(long base, long exp) {
	if(exp > 1024 * 32)
		throw string{"wow, that's quite the exponent you have there"};

	long result = 1;
	for(long i = 0; i < exp; ++i)
		result = result * base;

	return result;
}

double math_sqrt(double arg) {
	return sqrt(arg);
}

