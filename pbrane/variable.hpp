#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <string>
#include <vector>
#include <map>
#include "permission.hpp"

std::vector<std::string> getList(std::map<std::string, std::string> vars, std::string variable);
std::vector<std::string> makeList(std::string lists);

#endif // VARIABLE_HPP
