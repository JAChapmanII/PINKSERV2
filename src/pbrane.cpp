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
#include "modules.hpp"
#include "util.hpp"
using util::contains;
using util::fromString;
using util::split;
#include "markov.hpp"
#include "eventsystem.hpp"
#include "expressiontree.hpp"

string evaluate(string script, string nick);


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
	global::secondaryInit();

	EventSystem eventSystem;

	// while there is more input coming
	int done = 0;
	while(!cin.eof() && !done) {
		// read the current line of input
		string line;
		getline(cin, line);

		// TODO: logging

		vector<string> fields = split(line);
		if(fields[1] == (string)"PRIVMSG") {
			string nick = fields[0].substr(1, fields[0].find("!") - 1);

			size_t mstart = line.find(":", 1);
			string message = line.substr(mstart + 1);

			string target = fields[2];
			if(fields[2] == global::vars["bot.nick"].toString())
				target = nick;

			// if we recieve the restart command, restart
			if(message == (string)":!restart" && isOwner(nick))
				return 0;

			// if the line is a ! command, run it
			if(message[0] == '!')
				send(target, evaluate(message, nick), true);

			// if the line is a : invocation, evaluate it
			if(message[0] == ':')
				send(target, evaluate(message.substr(1), nick), true);

			// otherwise, run on text triggers
		}
		if(fields[1] == (string)"JOIN") {
			;// run join triggers
		}
		if(fields[1] == (string)"NICK") {
			;// run nick triggers
		}
		if((fields[1] == (string)"PART") || (fields[1] == (string)"QUIT")) {
			;// run leave triggers
		}
	}

	// free memory associated with modules
	modules::deinit(config::brainFileName);

	// deinit global
	global::deinit();

	return done - 1;
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
		result = nick + ": severe error: " + s;
	}
	return result;
}


