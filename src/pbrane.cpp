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

unsigned const maxLineLength = 512;

int main(int argc, char **argv) {
	srand(time(NULL));
	if(argc > 1) {
		stringstream ss;
		ss << argv[1];
		int seed;
		ss >> seed;
		srand(seed);
	}
	const string logFileName = "PINKSERV2.log",
			chatLogFileName = "PINKSERV2.chat",
			errorLogFileName = "PINKSERV2.err",
			myNick = "PINKSERV2",
			todoFileName = "TODO",
			ownerNick = "jac", channelName = "#pokengine";

	const string privmsgRegexExp =
		"^:([A-Za-z0-9_]*)!([-/@~A-Za-z0-9_\\.]*) PRIVMSG ([#A-Za-z0-9_]*) :(.*)";
	const string joinRegexExp =
		"^:([A-Za-z0-9_]*)!([-/@~A-Za-z0-9_\\.]*) JOIN :?([#A-Za-z0-9_]*)";
	const string toUsRegexExp = "^(" + myNick + "[L:\\,]?\\s+).*";
	const string toUsRRegexExp = "^(" + myNick + "[L:\\,]?\\s+)";
	const string helpRegexExp = "^\\s*help(\\s+(\\S+))?";

	modules::init(config::brainFileName);
	cerr << "loaded: ";
	for(auto module : modules::map)
		cerr << module.second->name() << " ";
	cerr << endl;

	// create primary regex objects {{{
	regex privmsgRegex(privmsgRegexExp, regex::perl);
	regex joinRegex(privmsgRegexExp, regex::perl);
	regex toUsRegex(toUsRegexExp, regex::perl);
	regex toUsRRegex(toUsRRegexExp, regex::perl);
	regex helpRegex(helpRegexExp, regex::perl);
	// }}}

	ofstream log(logFileName, fstream::app);
	if(!log.good()) {
		cerr << "Could not open log!" << endl;
		return 1;
	}
	ofstream chatLog(chatLogFileName, fstream::app);
	if(!chatLog.good()) {
		cerr << "Could not open chat log!" << endl;
		return 1;
	}
	ofstream errorLog(errorLogFileName, fstream::app);
	if(!errorLog.good()) {
		cerr << "Could not open error log!" << endl;
		return 1;
	}

	log << myNick << " started." << endl;
	map<string, int> siMap;

	// while there is more input coming
	int done = 0;
	while(!cin.eof() && !done) {
		// read the current line of input
		string line;
		getline(cin, line);

		smatch matches;
		// if the current line is a PRIVMSG...
		if(regex_match(line, matches, privmsgRegex)) {
			// log all the things!
			chatLog << matches[1] << "@" << matches[3] << ": " << matches[4] << endl;
			string message(matches[4]), nick(matches[1]), omessage(matches[4]);

			bool toUs = false;
			if(regex_match(message, toUsRegex)) {
				message = regex_replace(message, toUsRRegex, (string)"");
				toUs = true;
				chatLog << "  umsg: " << message << endl;
			}

			string rtarget = matches[3];
			if(rtarget == myNick) {
				rtarget = matches[1];
				toUs = true;
			}

			// start out by trying to match the reload command
			if(toUs && message == (string)"reload") {
				done = 78;
				break;
			} else if(toUs && regex_match(message, matches, helpRegex)) {
				string function = matches[2], res = "(null)";
				if(function.empty()) {
					stringstream ss;
					for(auto i : modules::map)
						ss << i.second->name() << ", ";
					res = ss.str();
					res = res.substr(0, res.length() - 2);
				} else {
					if(modules::map.find(function) == modules::map.end()) {
						res = "That function does not exist";
						for(auto i : modules::map) {
							if(i.second->name() == function) {
								res = i.second->help();
								break;
							}
						}
					} else {
						res = modules::map[function]->help();
					}
				}
				log << matches[1] << "@" << matches[3] << ": " << matches[4] << endl;
				log << " -> " << rtarget << " :" << nick << ": " << res << endl;
				cout << "PRIVMSG " << rtarget << " :" << nick << ": " << res << endl;
			} else {
				// setup function arguments
				FunctionArguments fargs;
				fargs.nick = nick;
				fargs.user = matches[2];
				fargs.target = matches[3];
				fargs.message = message;
				fargs.toUs = toUs;
				fargs.siMap = &siMap;
				if(fargs.nick == ownerNick)
					fargs.fromOwner = true;

				bool matched = false;
				// loop through setup modules trying to match their regex
				if(!contains(global::ignoreList, fargs.nick)) {
					for(auto mod : modules::map) {
						regex cmodr(mod.second->regex(), regex::perl);
						// if this module matches
						if(regex_match(message, fargs.matches, cmodr, match_extra)) {
							// log that we got a hit
							log << "module matched: " << mod.first << endl;
							// run the module
							string res = mod.second->run(fargs);
							if(res.empty()) {
								log << "module returned nothing, moving on" << endl;
							} else {
								if(res.length() > maxLineLength)
									res = res.substr(0, maxLineLength);
								// log the output/send the output
								log << matches[1] << "@" << matches[3] << ": " << matches[4] << endl;
								log << " -> " << rtarget << " :" << res << endl;
								cout << "PRIVMSG " << rtarget << " :" << res << endl;
								matched = true;
								break;
							}
						}
					}
					if(!matched) {
						for(auto mod : modules::map) {
							string res = mod.second->secondary(fargs);
							if(!res.empty()) {
								log << "module (secondary) matched " << mod.first << endl;
								if(res.length() > maxLineLength)
									res = res.substr(0, maxLineLength);

								log << matches[1] << "@" << matches[3] << ": " << matches[4] << endl;
								log << " -> " << rtarget << " :" << res << endl;
								cout << "PRIVMSG " << rtarget << " :" << res << endl;
								matched = true;
								break;
							}
						}
					}
				}

				if(!matched) {
					global::lastLog.push_back(global::ChatLine(nick, omessage));
					regex yesCommand(modules::map["yes"]->regex(), regex::perl);
					if(!contains(global::ignoreList, fargs.nick) && (toUs ||
								regex_match(message, fargs.matches, yesCommand,
									match_extra))) {
						log << matches[1] << "@" << matches[3] << ": " << matches[4] << endl;
						log << " -> " << rtarget << " :yes" << endl;
						cout << "PRIVMSG " << rtarget << " :yes" << endl;
					}
					//insert(message);
				}

				for(auto module : modules::map)
					module.second->passive(global::ChatLine(nick, matches[4]), matched);
			}
		// if the current line is a JOIN...
		} else if(regex_match(line, matches, joinRegex)) {
			// log all the join messages
			chatLog << matches[1] << " (" << matches[2] << ") has joined "
				<< matches[3] << endl;
		// otherwise...
		} else {
			// log all the failures
			errorLog << "no match: " << line << endl;
		}
	}

	// free memory associated with modules
	modules::deinit(config::brainFileName);

	return done - 1;
}

