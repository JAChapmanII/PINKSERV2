#ifndef MODULES_HPP
#define MODULES_HPP

#include <map>
#include <vector>
#include <string>
#include <fstream>

namespace modules {
	typedef std::string (*Function)(std::vector<std::string>);
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

	bool moduleLoaded(std::string mname);

	bool init(std::string brainFileName);
	bool deinit(std::string brainFileName);
}

#endif // MODULES_HPP
