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

namespace global {
	extern std::ofstream log;
	extern std::ofstream err;
	extern std::mt19937_64 rengine;
	extern Dictionary<std::string, unsigned> dictionary;
	extern EventSystem eventSystem;

	// variable, function map
	extern std::map<std::string, Variable> vars;

	// local variable map
	extern std::map<std::string, std::map<std::string, Variable>> lvars;

	bool init(unsigned int seed);
	void secondaryInit();
	bool deinit();

	extern std::vector<std::string> ignoreList;
	extern unsigned minSpeakTime;

	void send(std::string target, std::string line, bool send = true);

	bool isOwner(std::string nick);
	bool isAdmin(std::string nick);

	// get current time
	uint64_t now();
}

#endif // GLOBAL_HPP
