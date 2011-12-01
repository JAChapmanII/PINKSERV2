#include <iostream>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;
using std::getline;

#include <boost/regex.hpp>
using boost::regex;
using boost::smatch;
using boost::regex_match;

#include <fstream>
using std::ofstream;

#include <map>
using std::map;

// Structure used to pass relavent data to Functions {{{
struct FunctionArguments {
	smatch matches;
	string nick;
	string user;
	string target;
	string message;

	FunctionArguments() :
			matches(), nick(), user(), target(), message() {
	}
}; // }}}

// Base class for all other functions {{{
class Function {
	public:
		virtual ~Function() {}

		virtual string run(FunctionArguments fargs) {
			return "";
		}

		virtual string name() const {
			return "Base function";
		}
		virtual string help() const {
			return "Base function; cannot be invoked";
		}
		virtual string regex() const {
			return "^$";
		}
		virtual void reset() {
		}
}; // }}}


// A function to wave to people {{{
class WaveFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			if(fargs.message == (string)"o/")
				return "\\o";
			else
				return "o/";
		}

		virtual string name() const {
			return "wave";
		}
		virtual string help() const {
			return "Takes no arguments; waves.";
		}
		virtual string regex() const {
			return "^(o/|\\o)";
		}
}; // }}}

int main(int argc, char **argv) {
	const string logFileName = "pbrane.log", myNick = "pbrane";
	const string privmsgRegexExp =
		"^:([A-Za-z0-9_]*)!([-@~A-Za-z0-9_\\.]*) PRIVMSG ([#A-Za-z0-9_]*) :(.*)";
	const string joinRegexExp =
		"^:([A-Za-z0-9_]*)!([-@~A-Za-z0-9_\\.]*) JOIN :([#A-Za-z0-9_]*)";
	const string reloadRegexExp = myNick + "[:,]? +reload";

	map<string, Function *> moduleMap;
	moduleMap["wave"] = new WaveFunction();

	regex privmsgRegex(privmsgRegexExp, regex::perl);
	regex joinRegex(privmsgRegexExp, regex::perl);
	regex reloadRegex(reloadRegexExp, regex::perl);

	ofstream log("pbrane.log");
	if(!log.good()) {
		cerr << "Could not open log!" << endl;
		return 1;
	}

	// while there is more input coming
	while(!cin.eof()) {
		// read the current line of input
		string line;
		getline(cin, line);

		smatch matches;
		// if the current line is a PRIVMSG...
		if(regex_match(line, matches, privmsgRegex)) {
			// log all the things!
			log << "pmsg: " << matches[0] << endl;
			log << "  nick: " << matches[1] << endl;
			log << "  user: " << matches[2] << endl;
			log << "target: " << matches[3] << endl;
			log << "   msg: " << matches[4] << endl;
			string message(matches[4]);

			// start out by trying to match the reload command
			if(regex_match(message, reloadRegex)) {
				return 77;
			} else {
				// setup function arguments
				FunctionArguments fargs;
				fargs.nick = matches[1];
				fargs.user = matches[2];
				fargs.target = matches[3];
				fargs.message = message;

				// loop through setup modules trying to match their regex
				for(auto mod = moduleMap.begin(); mod != moduleMap.end(); ++mod) {
					regex cmodr(mod->second->regex(), regex::perl);
					// if this module matches
					if(regex_match(message, fargs.matches, cmodr)) {
						// log that we got a hit
						log << "module matched: " << mod->first << endl;
						// run the module
						string res = mod->second->run(fargs);
						// log the output/send the output
						log << " -> #uakroncs :" << res << endl;
						cout << "PRIVMSG #uakroncs :" << res << endl;
						break;
					}
				}
			}
		// if the current line is a JOIN...
		} else if(regex_match(line, matches, joinRegex)) {
			// log all the join messages
			log << matches[1] << " (" << matches[2] << ") has joined "
				<< matches[3] << endl;
		// otherwise...
		} else {
			// log all the failures
			log << "no match: " << line << endl;
		}
	}

	return 0;
}

