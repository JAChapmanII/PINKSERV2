#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <string>
#include <vector>
#include <map>
#include "dictionary.hpp"
#include "variable.hpp"
#include "varstore.hpp"
#include "journal.hpp"

namespace global {
	extern std::vector<std::string> moduleFunctionList;

	extern std::vector<std::string> ignoreList;
	extern unsigned minSpeakTime;

}

#endif // GLOBAL_HPP
