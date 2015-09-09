#ifndef MODULES_HPP
#define MODULES_HPP

#include <map>
#include <vector>
#include <string>
#include "variable.hpp"

namespace modules {
	typedef Variable (*Function)(std::vector<Variable>);
	struct Module {
		std::string name;
		std::string desc;
		bool loaded;
	};

	extern std::map<std::string, Function> hfmap;
	extern std::vector<Module> modules;

	bool init();
}

#endif // MODULES_HPP
