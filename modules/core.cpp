#include "core.hpp"
using std::string;
using boost::smatch;
using global::ChatLine;

#include <ctime>

#include "global.hpp"
#include "util.hpp"
using util::join;
using util::contains;
using util::fromString;
using util::asString;
#include "modules.hpp"


string IgnoreFunction::run(ChatLine line, smatch matches) { // {{{
	if(!global::isOwner(line.nick) && !global::isAdmin(line.nick))
		return "";

	string nstring = matches[1], nick = matches[2];
	if(nstring.empty() && nick.empty()) {
		if(global::ignoreList.empty())
			return line.nick + ": not ignoring anyone";
		return line.nick + ": " + join(global::ignoreList);
	}

	if(nstring.empty()) {
		if(contains(global::ignoreList, nick))
			return ""; //fargs.nick + ": " + nick + " already ignored";

		global::ignoreList.push_back(nick);
		return line.nick + ": ignored " + nick;
	}

	if(!contains(global::ignoreList, nick))
		return line.nick + ": user isn't ignored currently";

	auto it = find(global::ignoreList.begin(), global::ignoreList.end(), nick);
	if(*it != nick)
		return line.nick + ": error, it not nick!?";

	global::ignoreList.erase(it);
	return line.nick + ": " + nick + " no longer ignored ";
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


string HelpFunction::run(ChatLine line, smatch matches) { // {{{
	string func = matches[2];
	if(func.empty()) {
		string res;
		for(auto i : modules::map)
			res += i.second->name() + ", ";
		return res.substr(0, res.length() - 2);
	}

	if(contains(modules::map, func))
		return line.nick + ": " + modules::map[func]->help();
	for(auto i : modules::map) {
		if(i.second->name() == func)
			return line.nick + ": " + i.second->help();
	}

	return line.nick + ": that function does not exist";
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


string ShutupFunction::run(ChatLine line, smatch matches) { // {{{
	unsigned t = 30;
	string num = matches[2];
	if(!num.empty())
		t = fromString<unsigned>(num);
	if(t > 180)
		t = 180;

	global::minSpeakTime = time(NULL) + t*60;
	return line.nick + ": yes sir, shutting up for " + asString(t) + " minutes";
} // }}}
string ShutupFunction::name() const { // {{{
	return "quiet";
} // }}}
string ShutupFunction::help() const { // {{{
	return (string)"Make me quiet for a 30 minutes or a user specified amount "
		+ "of time [ex: !quiet 60]";
} // }}}
string ShutupFunction::regex() const { // {{{
	return "^!quiet(\\s+(\\d+))?";
} // }}}


string UnShutupFunction::run(ChatLine line, smatch matches) { // {{{
	global::minSpeakTime = time(NULL) - 1;
	return line.nick + ": OK! :D";
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

