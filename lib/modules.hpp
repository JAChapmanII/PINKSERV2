#ifndef MODULES_HPP
#define MODULES_HPP

#include <map>
#include <string>
#include "function.hpp"

namespace modules {
	extern std::map<std::string, Function *> map;

	bool init(std::string fileName);
	bool deinit(std::string fileName);
}

#endif // MODULES_HPP
