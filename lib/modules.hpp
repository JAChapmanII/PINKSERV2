#ifndef MODULES_HPP
#define MODULES_HPP

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include "variable.hpp"

namespace modules {
	typedef Variable (*Function)(std::vector<Variable>);
	typedef void (*LoadFunction)(std::istream &);
	typedef void (*SaveFunction)(std::ostream &);
	struct Module {
		std::string name;
		std::string desc;
		LoadFunction load;
		SaveFunction save;
		bool loaded;
	};

	extern std::map<std::string, Function> hfmap;
	extern std::vector<Module> modules;

	bool init(std::string brainFileName);
	bool deinit(std::string brainFileName);
}

#endif // MODULES_HPP
