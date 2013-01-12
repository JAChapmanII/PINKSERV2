#include <iostream>
using std::cin;
using std::cerr;
using std::endl;

#include <string>
using std::string;
using std::getline;

#include <random>
using std::random_device;

#include <boost/regex.hpp>
using boost::regex;
using boost::smatch;
using boost::regex_match;
using boost::match_extra;

#include "global.hpp"
using global::isOwner;
#include "config.hpp"
#include "modules.hpp"
#include "util.hpp"
using util::contains;
using util::fromString;
#include "markov.hpp"

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
	global::log << "----- " << config::nick << " started -----" << endl;
	cerr << "----- " << config::nick << " started -----" << endl;

	regex privmsgRegex(config::regex::privmsg, regex::perl);
	regex joinRegex(config::regex::join, regex::perl);

	modules::init(config::brainFileName);
	global::secondaryInit();

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
			if(isOwner(nick) && (message == config::reload)) {
				global::log << "----- RESTARTING -----" << endl;
				global::log.flush();
				done = 78;
				break;
			}
			// next try to just die
			if(isOwner(nick) && (message == config::die)) {
				global::log << "----- DIEING -----" << endl;
				return 77;
			}

			// TODO: make this simpler
			observe(line);

			if(message.empty()) {
				global::err << "main: message empty" << endl;
			} else {
				;// TODO: LogItem::parse, create ExpressionTree and evaluate
			}
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

