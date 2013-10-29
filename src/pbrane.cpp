#include <iostream>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;
using std::getline;

#include <vector>
using std::vector;

#include <random>
using std::random_device;

#include "global.hpp"
using global::isOwner;
using global::send;
#include "config.hpp"
#include "journal.hpp"
#include "modules.hpp"
#include "util.hpp"
using util::contains;
using util::fromString;
using util::asString;
using util::split;
using util::startsWith;
using util::trim;
#include "markov.hpp"
#include "eventsystem.hpp"
#include "expressiontree.hpp"
#include "events.hpp"

void process(string script, string nick, string target);
string evaluate(string script, string nick);

struct PrivateMessage {
	string message;
	string nick;
	string target;
};
typedef bool (*hook)(PrivateMessage pmsg);

bool powerHook(PrivateMessage pmsg);
bool regexHook(PrivateMessage pmsg);

vector<hook> hooks = { &powerHook, &regexHook };

vector<string> noInterpret = { ":p", ":P", ":)", ":(", ":|", ":]", ":[" };
bool notBlacklisted(string s);
bool notBlacklisted(string s) {
	for(auto b : noInterpret)
		if(s == b)
			return false;
	return true;
}

bool canEvaluate(string message);
bool canEvaluate(string message) {
	if(!notBlacklisted(message))
		return false;
	if(message.front() == ':') {
		if(message.find(';') != string::npos)
			return true;
		if(message.find("!") != string::npos)
			return true;
	}
	return false;
}

int main(int argc, char **argv) {
	unsigned int seed = 0;
	if(argc > 1) {
		seed = fromString<unsigned int>(argv[1]);
	} else {
		random_device randomDevice;
		seed = randomDevice();
	}

	if(!global::init(seed)) {
		cerr << "pbrane: global::init failed" << endl;
		return -1;
	}

	// TODO: don't hard-code these. These should be set in the startup file
	global::vars["bot.nick"] = "pbrane_future";
	global::vars["bot.maxLineLength"] = "256";
	global::vars["bot.owner"] = "jac";

	if(!global::secondaryInit()) {
		cerr << "pbrane: global::secondaryInit failed" << endl;
		// TODO: this should fail out completely?
	}


	global::log << "----- " << global::vars["bot.nick"].toString() << " started -----" << endl;
	cerr << "----- " << global::vars["bot.nick"].toString() << " started -----" << endl;

	modules::init(config::brainFileName);
	global::secondaryInit(); // TODO: we do this twice?
	journal::init();

	// while there is more input coming
	global::done = false;
	while(!cin.eof() && !global::done) {
		// read the current line of input
		string line;
		getline(cin, line);

		if(line.empty())
			continue;
		journal::Entry entry(line);
		journal::push(entry);
		global::log << entry.format() << endl;

		vector<string> fields = split(line);
		if(fields[1] == (string)"PRIVMSG") {
			string nick = fields[0].substr(1, fields[0].find("!") - 1);

			size_t mstart = line.find(":", 1);
			string message = line.substr(mstart + 1);

			string target = fields[2];
			if(fields[2] == global::vars["bot.nick"].toString())
				target = nick;

			// if the line is a ! command, run it
			//if(message[0] == '!')
				//process(message, nick, target);
			// if the line is a : invocation, evaluate it
			/*else*/ if(canEvaluate(message))
				process(message.substr(1), nick, target);
			// otherwise, run on text triggers
			else {
				// TODO: proper environment for triggers
				global::vars["nick"] = nick;
				global::vars["text"] = message;

				vector<Variable> results = global::eventSystem.process(EventType::Text);
				if(results.size() == 1)
					send(target, results.front().toString(), true);
			}
		}
		if(fields[1] == (string)"JOIN") {
			string nick = fields[0].substr(1, fields[0].find("!") - 1);
			string where = fields[2];
			if(where[0] == ':')
				where = where.substr(1);

			// TODO: proper environment for triggers
			global::vars["nick"] = nick;
			global::vars["where"] = where;

			vector<Variable> results = global::eventSystem.process(EventType::Join);
			if(results.size() == 1)
				send(where, results.front().toString(), true);
		}
		if(fields[1] == (string)"NICK") {
			;// run nick triggers
		}
		if((fields[1] == (string)"PART") || (fields[1] == (string)"QUIT")) {
			;// run leave triggers
		}
	}

	journal::deinit();

	// free memory associated with modules
	modules::deinit(config::brainFileName);

	// deinit global
	global::deinit();

	return 0;
}

void process(string script, string nick, string target) {
	// run special hooks first
	bool processed = false;
	for(auto h : hooks)
		if((*h)({ script, nick, target })) {
			processed = true;
			break;
		}
	if(processed)
		return;

	script = trim(script);
	if(script.empty())
		return;

	// assume we can run the script
	send(target, evaluate(script, nick), true);
}
string evaluate(string script, string nick) {
	string result;
	try {
		ExpressionTree *etree = ExpressionTree::parse(script);
		try {
			result = etree->evaluate(nick).toString();
		} catch(string &s) {
			result = nick + ": error: " + s;
		}
		delete etree;
	} catch(string &s) {
		result = nick + ": syntax error: " + s;
	}
	return result;
}


bool powerHook(PrivateMessage pmsg) {
	if(pmsg.message == (string)"!restart" && isOwner(pmsg.nick))
		return global::done = true;
	return false;
}
bool regexHook(PrivateMessage pmsg) {
	if(pmsg.message[0] != 's')
		return false;
	return false;
}

