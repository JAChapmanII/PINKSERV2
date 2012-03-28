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
	global::log(config::nick + " started.");

	regex privmsgRegex(config::regex::privmsg, regex::perl);
	regex joinRegex(config::regex::join, regex::perl);

	modules::init(config::brainFileName);
	string lstring = "loaded: ";
	for(auto module : modules::map)
		lstring += module.second->name() + " ";
	global::log(lstring);

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
				global::log("restarting -----------");
				done = 78;
				break;
			}
			// next try to just die
			if(message == config::die) {
				global::log("dieing ---------------");
				return 77;
			}

			global::parse(global::ChatLine(nick, target, message));
		// if the current line is a JOIN...
		} else if(regex_match(line, matches, joinRegex)) {
			// log all the join messages
			global::log(matches[1] + " (" + matches[2] + ") has joined " + matches[3]);
		// otherwise...
		} else {
			// log all the failures
			global::err("no match: " + line);
		}
	}

	// free memory associated with modules
	modules::deinit(config::brainFileName);

	return done - 1;
}

