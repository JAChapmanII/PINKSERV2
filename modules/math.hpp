#ifndef MODULES_MATH_HPP
#define MODULES_MATH_HPP

#include <vector>
#include "variable.hpp"

// #m: math: math functions

// #f: math_pow = pow: raise a number to a power
Variable math_pow(std::vector<Variable> arguments);

// #f: math_sqrt = sqrt: take the square root of a number
Variable math_sqrt(std::vector<Variable> arguments);

#endif // MODULES_MATH_HPP
