#include "core.hpp"
using std::string;

#include "global.hpp"
#include "util.hpp"
using util::join;
using util::contains;

string IgnoreFunction::run(FunctionArguments fargs) {
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
			return fargs.nick + ": " + nick + " already ignored";

		global::ignoreList.push_back(nick);
		return fargs.nick + ": ignored " + nick;
	}

	if(!contains(global::ignoreList, nick))
		return fargs.nick + ": user isn't ignored currently";

	auto it = find(global::ignoreList.begin(), global::ignoreList.end(), nick);
	if(*it != nick)
		return fargs.nick + ": erorr, it not nick!?";

	global::ignoreList.erase(it);
	return fargs.nick + ": " + nick + " no longer ignored ";
}

string IgnoreFunction::name() const {
	return "ignore";
}
string IgnoreFunction::help() const {
	return "Ignore a user";
}
string IgnoreFunction::regex() const {
	return "^!ignore(?:\\s+(!)?(\\S+))?";
}

