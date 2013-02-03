#include <iostream>
using std::cin;
using std::cerr;
using std::endl;

#include <string>
using std::string;
using std::getline;

#include <random>
using std::random_device;

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
	global::log << "----- " << global::vars["bot.nick"].toString() << " started -----" << endl;
	cerr << "----- " << global::vars["bot.nick"].toString() << " started -----" << endl;

	modules::init(config::brainFileName);
	global::secondaryInit();

	// while there is more input coming
	int done = 0;
	while(!cin.eof() && !done) {
		// read the current line of input
		string line;
		getline(cin, line);

		// TODO: LogItem::parse, switch on type
		// if the current line is a PRIVMSG...
		/*if(is text) {
			// create ExpressionTree and evaluate

			// TODO: must do restart and sleep here?
			// TODO: set global::done to 78 on reload, 77 on sleep?

			if(message.empty()) {
				global::err << "main: message empty" << endl;
			} else {
			}
		// if the current line is a JOIN...
		} else if(is join) {
			// log all the join messages
			global::log << matches[1] << " (" << matches[2] << ")"
				<< " has joined " << matches[3] << endl;
		// otherwise...
		} else {
			// log all the failures
			global::err << "NO MATCH: " + line << endl;;
		}*/
	}

	// free memory associated with modules
	modules::deinit(config::brainFileName);

	// deinit global
	global::deinit();

	return done - 1;
}

