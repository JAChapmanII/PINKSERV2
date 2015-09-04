#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include "dictionary.hpp"
#include "variable.hpp"
#include "eventsystem.hpp"
#include "db.hpp"
#include "varstore.hpp"

namespace global {
	extern bool done;
	extern std::ofstream log;
	extern std::mt19937_64 rengine;
	extern Dictionary dictionary;
	extern std::vector<std::string> moduleFunctionList;

	extern zidcu::Database db;

	// variable, function map
	extern VarStore vars;

	// local variable map
	extern std::map<std::string, std::map<std::string, Variable>> lvars;

	bool init(unsigned int seed);
	bool secondaryInit();
	bool deinit();

	extern std::vector<std::string> ignoreList;
	extern unsigned minSpeakTime;

	void send(std::string network, std::string target, std::string line,
			bool send = true);

	bool isOwner(std::string nick);
	bool isAdmin(std::string nick);

	// get current time
	long long now();

	// debug flags
	extern bool debugSQL;
	extern bool debugEventSystem;
	extern bool debugFunctionBody;
}

#endif // GLOBAL_HPP
