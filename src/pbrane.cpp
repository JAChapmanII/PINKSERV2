// std includes {{{
#include <iostream>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;

using std::skipws;

#include <string>
using std::string;
using std::getline;

#include <boost/regex.hpp>
using boost::regex;
using boost::smatch;
using boost::regex_match;
using boost::match_extra;

#include <fstream>
using std::ofstream;
using std::ifstream;
using std::fstream;

#include <map>
using std::map;

#include <sstream>
using std::stringstream;

#include <vector>
using std::vector;
// }}}

#include "global.hpp"
#include "config.hpp"
#include "util.hpp"
using util::contains;
#include "modules.hpp"
#include "function.hpp"

int main(int argc, char **argv) {
	srand(time(NULL));
	if(argc > 1) {
		stringstream ss;
		ss << argv[1];
		int seed;
		ss >> seed;
		srand(seed);
	}
	if(!global::init()) {
		cerr << "pbrane: global::init failed" << endl;
		return -1;
	}
	global::log << "-----" << config::nick << " started -----" << endl;

	regex privmsgRegex(config::regex::privmsg, regex::perl);
	regex joinRegex(config::regex::join, regex::perl);

	modules::init(config::brainFileName);
	global::log << "loaded modules: ";
	for(auto module : modules::map)
		global::log << module.second->name() << " ";
	global::log << endl;;

	// while there is more input coming
	int done = 0;
	while(!cin.eof() && !done) {
		// read the current line of input
		string line;
		getline(cin, line);

		smatch matches;
		// if the current line is a PRIVMSG...
		if(regex_match(line, matches, privmsgRegex)) {
			string nick(matches[1]), target(matches[3]), message(matches[4]);

			// start out by trying to match the reload command
			if(message == config::reload) {
				global::log << "----- RESTARTING -----" << endl;
				global::log.flush();
				done = 78;
				break;
			}
			// next try to just die
			if(message == config::die) {
				global::log << "----- DIEING -----" << endl;
				return 77;
			}

			// TODO: un awful hack this
			// If Pokengine is the user speaking for someone on the MMO, attempt
			// to translate the message to be what it appears as in the MMO.
			if((nick == "Pokengine") && (message[0] == '<')) {
				nick = message.substr(1);
				nick = nick.substr(0, nick.find(">"));
				message = message.substr(message.find(">") + 1);
				message = message.substr(message.find_first_not_of(" \t\r\n"));
			}

			global::parse(global::ChatLine(nick, target, message));
		// if the current line is a JOIN...
		} else if(regex_match(line, matches, joinRegex)) {
			// log all the join messages
			global::log << matches[1] << " (" << matches[2] << ")"
				<< " has joined " << matches[3] << endl;
		// otherwise...
		} else {
			// log all the failures
			global::err << "NO MATCH: " + line << endl;;
		}
	}

	// free memory associated with modules
	modules::deinit(config::brainFileName);

	// deinit global
	global::deinit();

	return done - 1;
}

