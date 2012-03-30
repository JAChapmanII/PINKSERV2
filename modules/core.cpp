#include "core.hpp"
using std::string;

#include <sstream>
using std::stringstream;

#include <ctime>

#include "global.hpp"
#include "util.hpp"
using util::join;
using util::contains;
#include "modules.hpp"

string IgnoreFunction::run(FunctionArguments fargs) { // {{{
	if(!fargs.fromOwner)
		return "";

	string nstring = fargs.matches[1], nick = fargs.matches[2];
	if(nstring.empty() && nick.empty()) {
		if(global::ignoreList.empty())
			return fargs.nick + ": not ignoring anyone";
		return fargs.nick + ": " + join(global::ignoreList);
	}

	if(nstring.empty()) {
		if(contains(global::ignoreList, nick))
			return ""; //fargs.nick + ": " + nick + " already ignored";

		global::ignoreList.push_back(nick);
		return fargs.nick + ": ignored " + nick;
	}

	if(!contains(global::ignoreList, nick))
		return fargs.nick + ": user isn't ignored currently";

	auto it = find(global::ignoreList.begin(), global::ignoreList.end(), nick);
	if(*it != nick)
		return fargs.nick + ": error, it not nick!?";

	global::ignoreList.erase(it);
	return fargs.nick + ": " + nick + " no longer ignored ";
} // }}}

string IgnoreFunction::name() const { // {{{
	return "ignore";
} // }}}
string IgnoreFunction::help() const { // {{{
	return "Ignore a user";
} // }}}
string IgnoreFunction::regex() const { // {{{
	return "^!ignore(?:\\s+(!)?(\\S+))?";
} // }}}


string HelpFunction::run(FunctionArguments fargs) { // {{{
	string func = fargs.matches[2];
	if(func.empty()) {
		string res;
		for(auto i : modules::map)
			res += i.second->name() + ", ";
		return res.substr(0, res.length() - 2);
	}

	if(contains(modules::map, func))
		return fargs.nick + ": " + modules::map[func]->help();
	for(auto i : modules::map) {
		if(i.second->name() == func)
			return fargs.nick + ": " + i.second->help();
	}

	return fargs.nick + ": that function does not exist";
} // }}}
string HelpFunction::name() const { // {{{
	return "help";
} // }}}
string HelpFunction::help() const { // {{{
	return "Do you really need to ask?";
} // }}}
string HelpFunction::regex() const { // {{{
	return "^!help(\\s+(\\S+)?)?";
} // }}}


string ShutupFunction::run(FunctionArguments fargs) { // {{{
	unsigned t = 5;
	string num = fargs.matches[2];
	stringstream ss;
	if(!num.empty()) {
		ss << num;
		ss >> t;
		if(t > 60)
			t = 60;
		ss.clear();
	}
	global::minSpeakTime = time(NULL) + t*60;
	ss << fargs.nick << ": right-y-oh! shutting up for " << t << " minutes";
	return ss.str();
} // }}}
string ShutupFunction::name() const { // {{{
	return "quiet";
} // }}}
string ShutupFunction::help() const { // {{{
	return "Make me quiet for a few minutes";
} // }}}
string ShutupFunction::regex() const { // {{{
	return "^!quiet(\\s+(\\d+))?";
} // }}}


string UnShutupFunction::run(FunctionArguments fargs) { // {{{
	global::minSpeakTime = time(NULL) - 1;
	return fargs.nick + ": OK! :D";
} // }}}
string UnShutupFunction::name() const { // {{{
	return "unquiet";
} // }}}
string UnShutupFunction::help() const { // {{{
	return "Get me to talk again";
} // }}}
string UnShutupFunction::regex() const { // {{{
	return "!unquiet";
} // }}}

