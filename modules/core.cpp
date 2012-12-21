#include "core.hpp"
using std::string;
using std::ostream;
using std::istream;
using boost::smatch;

#include <ctime>

#include "global.hpp"
#include "util.hpp"
using util::join;
using util::contains;
using util::fromString;
using util::asString;
#include "modules.hpp"


IgnoreFunction::IgnoreFunction() : Function( // {{{
		"ignore", "Ignore a user", "^!ignore(?:\\s+(!)?(\\S+))?", true) {
}// }}}
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
ostream &IgnoreFunction::output(ostream &out) { // {{{
	unsigned char size = global::ignoreList.size();
	out << size;
	for(auto nick : global::ignoreList)
		brain::write(out, nick);
	return out;
}// }}}
istream &IgnoreFunction::input(istream &in) { // {{{
	int size = in.get();
	if(size < 0)
		return in;
	for(int i = 0; i < size; ++i) {
		string nick;
		brain::read(in, nick);
		global::ignoreList.push_back(nick);
	}
	return in;
} // }}}


HelpFunction::HelpFunction() : Function( // {{{
		"help", "Do you really need to ask?", "^!help(\\s+(\\S+)?)?", false) {
}// }}}
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


ShutupFunction::ShutupFunction() : Function( // {{{
		"quiet", "Make me quiet for a default or a user specified amount of time",
		"^!quiet(\\s+(\\d+))?", false) {
}// }}}
string ShutupFunction::run(ChatLine line, smatch matches) { // {{{
	unsigned t = 120;
	string num = matches[2];
	if(!num.empty())
		t = fromString<unsigned>(num);
	if(t > 180)
		t = 180;

	global::minSpeakTime = time(NULL) + t*60;
	return line.nick + ": yes sir, shutting up for " + asString(t) + " minutes";
} // }}}


UnShutupFunction::UnShutupFunction() : Function( // {{{
		"unquiet", "Get me to talk again", "!unquiet", false) {
}// }}}
string UnShutupFunction::run(ChatLine line, smatch matches) { // {{{
	global::minSpeakTime = time(NULL) - 1;
	return line.nick + ": OK! :D";
} // }}}


KickFunction::KickFunction() : Function( // {{{
		"kick", "Kick a user", "!kick(\\s+(\\w+))", false) {
}// }}}
string KickFunction::run(ChatLine line, smatch matches) { // {{{
	string user = matches[2];
	if(global::isOwner(line.nick) || global::isAdmin(line.nick)) {
		global::kick(line.target, user, line.nick + " said so");
		return line.nick + ": (tried to) kick " + user;
	}
	return line.nick + ": you are not authed to do that";
} // }}}

