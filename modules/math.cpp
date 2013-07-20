#include "math.hpp"
using std::vector;
using std::string;

#include <cmath>

Variable math_pow(vector<Variable> arguments) {
	if(arguments.size() != 2)
		throw (string)"error: pow takes two arguments";

	Variable base = arguments.front().asInteger(),
		result(1L, Permissions());
	long exponent = arguments.back().asInteger().value.l;

	for(long i = 0; i < exponent; ++i)
		result = result * base;

	return result;
}

Variable math_sqrt(vector<Variable> arguments) {
	if(arguments.size() != 1)
		throw (string)"error: sqrt takes one argument";

	long number = arguments.front().asInteger().value.l;
	return Variable(sqrt(number), Permissions());
}

