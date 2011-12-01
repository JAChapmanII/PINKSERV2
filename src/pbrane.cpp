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

#include <sstream>
using std::stringstream;

// Structure used to pass relavent data to Functions {{{
struct FunctionArguments {
	smatch matches;
	string nick;
	string user;
	string target;
	string message;
	bool toUs;

	map<string, int> *siMap;

	FunctionArguments() :
		matches(), nick(), user(), target(), message(), toUs(false), siMap(NULL) {
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
			if(fargs.message[0] == 'o')
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
			return "^(o/|\\\\o)( .*)?";
		}
}; // }}}

// A function to output some fish(es) {{{
class FishFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			int fcount = 1;
			if((fargs.message.length() >= 5) && (fargs.message[4] == 'e'))
				fcount = rand() % 6 + 2;

			stringstream ss;
			for(int i = 0; i < fcount; ++i) {
				if(rand() % 2)
					ss << "<><";
				else
					ss << "><>";
				if(i != fcount - 1)
					ss << string(rand() % 3 + 1, ' ');
			}
			return ss.str();
		}

		virtual string name() const {
			return "fish(es)?";
		}
		virtual string help() const {
			return "Takes no arguments; outputs fish(es).";
		}
		virtual string regex() const {
			return "^fish(es)?( .*)?";
		}
}; // }}}

// A function to provide artificial love {{{
class LoveFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			if(fargs.message[1] == '/')
				return ":(";
			return "<3";
		}

		virtual string name() const {
			return "love / <3";
		}
		virtual string help() const {
			return "Takes no arguments; outputs love.";
		}
		virtual string regex() const {
			return "^</?3( .*)?";
		}
}; // }}}

// Someone wants a train {{{
class TrainFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			int ccount = rand() % 8, dir = rand() % 2;
			stringstream ss;
			if(dir)
				ss << "/.==.]";
			else
				ss << "{. .}";
			for(int i = 0; i < ccount; ++i)
				ss << "[. .]";
			if(dir)
				ss << "{. .}";
			else
				ss << "[.==.\\";

			return ss.str();
		}

		virtual string name() const {
			return "sl";
		}
		virtual string help() const {
			return "Takes no arguments; returns a train.";
		}
		virtual string regex() const {
			return "^sl( .*)?";
		}
}; // }}}

// WUB WUB WUB WUB WUB {{{
class DubstepFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			return "WUB WUB WUB";
		}

		virtual string name() const {
			return "dubstep";
		}
		virtual string help() const {
			return "Takes no arguments; rocks.";
		}
		virtual string regex() const {
			return "^dubstep( .*)?";
		}
}; // }}}

// set a variable to something {{{
class SetFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			string varName = fargs.matches[1];
			stringstream is(fargs.matches[2]);
			int value = 0;
			is >> value;

			(*fargs.siMap)[varName] = value;

			stringstream ss("Set ");
			ss << varName << " to " << value;
			return ss.str();
		}

		virtual string name() const {
			return "set";
		}
		virtual string help() const {
			return "Sets a variable to be an integer";
		}
		virtual string regex() const {
			return "^\\s*set\\s+(\\w+)\\s+(\\d+).*";
		}
}; // }}}

// increment a variable {{{
class IncrementFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			// prefix operator
			string varName = fargs.matches[2];
			if(varName.empty())
				// postfix operator
				varName = fargs.matches[3];

			stringstream ss;
			ss << varName << " is now " << (++((*fargs.siMap)[varName]));
			return ss.str();
		}

		virtual string name() const {
			return "++";
		}
		virtual string help() const {
			return "Increment variable";
		}
		virtual string regex() const {
			return "^\\s*(\\+\\+\\s*(\\w+)|(\\w+)\\s*\\+\\+)( .*)?";
		}
}; // }}}
// decrement a variable {{{
class DecrementFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			// prefix operator
			string varName = fargs.matches[2];
			if(varName.empty())
				// postfix operator
				varName = fargs.matches[3];

			stringstream ss;
			ss << varName << " is now " << (--((*fargs.siMap)[varName]));
			return ss.str();
		}

		virtual string name() const {
			return "--";
		}
		virtual string help() const {
			return "Decrement variable";
		}
		virtual string regex() const {
			return "^\\s*(--\\s*(\\w+)|(\\w+)\\s*--)( .*)?";
		}
}; // }}}

// erase a variable {{{
class EraseFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			string varName = fargs.matches[1];

			int ecount = (*fargs.siMap).erase(varName);
			if(ecount == 0)
				return "Variable didn't exist anyway.";
			else
				return "Erased " + varName;
		}

		virtual string name() const {
			return "erase";
		}
		virtual string help() const {
			return "Erases a variable";
		}
		virtual string regex() const {
			return "^\\s*erase\\s+(\\w+)(\\s.*)?";
		}
}; // }}}

// Return one thing or the other {{{
class OrFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			boost::regex r(this->regex(), regex::perl);
			string choice = fargs.matches[rand() % 2 + 1];
			if(regex_match(choice, fargs.matches, r))
				return this->run(fargs);
			else
				return choice;
		}

		virtual string name() const {
			return "or";
		}
		virtual string help() const {
			return "Returns one of multiple possibilities";
		}
		virtual string regex() const {
			return "(.*)\\s+or\\s+(.*)";
		}
}; // }}}

int main(int argc, char **argv) {
	const string logFileName = "pbrane.log", myNick = "pbrane";
	const string privmsgRegexExp =
		"^:([A-Za-z0-9_]*)!([-@~A-Za-z0-9_\\.]*) PRIVMSG ([#A-Za-z0-9_]*) :(.*)";
	const string joinRegexExp =
		"^:([A-Za-z0-9_]*)!([-@~A-Za-z0-9_\\.]*) JOIN :([#A-Za-z0-9_]*)";
	const string toUsRegexExp = "^(" + myNick + "[:\\,]?\\s+).*";
	const string toUsRRegexExp = "^(" + myNick + "[:\\,]?\\s+)";

	srand(time(NULL));

	map<string, Function *> moduleMap;
	moduleMap["wave"] = new WaveFunction();
	moduleMap["fish"] = new FishFunction();
	moduleMap["love"] = new LoveFunction();
	moduleMap["train"] = new TrainFunction();
	moduleMap["wub"] = new DubstepFunction();
	moduleMap["or"] = new OrFunction();

	moduleMap["set"] = new SetFunction();
	moduleMap["++"] = new IncrementFunction();
	moduleMap["--"] = new DecrementFunction();
	moduleMap["erase"] = new EraseFunction();

	regex privmsgRegex(privmsgRegexExp, regex::perl);
	regex joinRegex(privmsgRegexExp, regex::perl);
	regex toUsRegex(toUsRegexExp, regex::perl);
	regex toUsRRegex(toUsRRegexExp, regex::perl);

	ofstream log("pbrane.log");
	if(!log.good()) {
		cerr << "Could not open log!" << endl;
		return 1;
	}

	log << "pbrane started." << endl;

	map<string, int> siMap;

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

			bool toUs = false;
			if(regex_match(message, toUsRegex)) {
				message = regex_replace(message, toUsRRegex, (string)"");
				toUs = true;
				log << "  umsg: " << message << endl;
			}

			string rtarget = matches[3];
			if(rtarget == myNick) {
				rtarget = matches[1];
				toUs = true;
			}

			// start out by trying to match the reload command
			if(toUs && message == (string)"reload") {
				return 77;
			} else {
				// setup function arguments
				FunctionArguments fargs;
				fargs.nick = matches[1];
				fargs.user = matches[2];
				fargs.target = matches[3];
				fargs.message = message;
				fargs.toUs = toUs;
				fargs.siMap = &siMap;

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
						log << " -> " << rtarget << " :" << res << endl;
						cout << "PRIVMSG " << rtarget << " :" << res << endl;
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

