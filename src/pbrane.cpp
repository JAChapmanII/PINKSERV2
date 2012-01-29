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

#include <map>
using std::map;

#include <sstream>
using std::stringstream;

#include <queue>
using std::queue;

#include <vector>
using std::vector;

#include <algorithm>
using std::find;

vector<string> lastLog;

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

		virtual bool canStore() {
			return false;
		}
		virtual vector<char> *unload() {
			return NULL;
		}
		virtual size_t load(vector<char> *from) {
			return 0;
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
// A function to provide artificial love {{{
class LoveFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			if(fargs.message[1] == '/')
				return ":(";
			return "<3";
		}

		virtual string name() const {
			return "</?3";
		}
		virtual string help() const {
			return "Takes no arguments; outputs love.";
		}
		virtual string regex() const {
			return "^</?3( .*)?";
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
			return "^!fish(es)?( .*)?";
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
			return "^!sl( .*)?";
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
			return "^(!dubstep|WUB|wub)( .*)?";
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

			stringstream ss;
			ss << "Set " << varName << " to " << value;
			return ss.str();
		}

		virtual string name() const {
			return "set";
		}
		virtual string help() const {
			return "Sets a variable to be an integer";
		}
		virtual string regex() const {
			return "^!set\\s+(\\w+)\\s+(\\d+).*";
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
			return "^!erase\\s+(\\w+)(\\s.*)?";
		}
}; // }}}

// list all variables {{{
class ListFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			stringstream ss;
			unsigned j = 0;
			for(auto i = (*fargs.siMap).begin(); i != (*fargs.siMap).end(); ++i, ++j) {
				ss << i->first;
				if(j != (*fargs.siMap).size() - 1)
					ss << ", ";
			}

			return ss.str();
		}

		virtual string name() const {
			return "list";
		}
		virtual string help() const {
			return "List stored variables";
		}
		virtual string regex() const {
			return "^!list(\\s.*)?";
		}
}; // }}}

// Return one thing or the other {{{
class OrFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			if(!fargs.toUs)
				return "";
			queue<string> q;
			q.push(fargs.matches[1]);
			q.push(fargs.matches[2]);

			vector<string> results;
			boost::regex r(this->regex(), regex::perl);
			smatch matches;
			while(!q.empty()) {
				string cur = q.front(); q.pop();
				if(regex_match(cur, matches, r)) {
					q.push(matches[1]);
					q.push(matches[2]);
				} else {
					results.push_back(cur);
				}
			}

			return results[rand() % results.size()];
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

map<string, map<string, unsigned>> markovModel;
const unsigned markovOrder = 2;

vector<string> split(string str, string on);
vector<string> split(string str, string on) {
	vector<string> results;
	size_t first = str.find_first_not_of(on);
	while(first != string::npos) {
		size_t last = str.find_first_of(on, first + 1);
		results.push_back(str.substr(first, last - first));
		if(last == string::npos)
			break;
		first = str.find_first_not_of(on, last + 1);
	}
	return results;
}

template<typename T> bool contains(vector<T> vec, T val);
template<typename T> bool contains(vector<T> vec, T val) {
	return (find(vec.begin(), vec.end(), val) != vec.end());
}

void insert(string text);
void markov_push(vector<string> words, unsigned order);
string fetch(string seed);

void markov_push(vector<string> words, unsigned order) {
	if(words.size() < order)
		return;

	// insert first sets of chains
	for(unsigned s = 0; s < words.size() - order; ++s) {
		string start = words[s];
		for(unsigned i = 1; i < order; ++i)
			start += (string)" " + words[s + i];
		markovModel[start][words[s + order]]++;
	}

	// insert last few words -> null mapping
	string start = words[words.size() - order];
	for(unsigned i = 1; i < order; ++i)
		start += (string)" " + words[words.size() - order + i];
	markovModel[start][""]++;
}
void insert(string text) {
	vector<string> words = split(text, " \t");
	for(unsigned o = 1; o <= markovOrder; ++o)
		markov_push(words, o);
}

string fetch(string seed) {
	if(markovModel[seed].empty())
		return "";
	unsigned total = 0;
	map<string, unsigned> seedMap = markovModel[seed];
	for(auto i = seedMap.begin(); i != seedMap.end(); ++i)
		total += i->second;
	unsigned r = rand() % total;
	auto i = seedMap.begin();
	while(r > i->second) {
		r -= i->second;
		++i;
	}
	return i->first;
}

// Handles returning markov chains {{{
class MarkovFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			string init = fargs.matches[1];
			vector<string> words = split(init, " \t");

			string start, next;
			do {
				start = "";
				if(words.size() < markovOrder) {
					start = words[0];
					for(unsigned i = 1; i < words.size(); ++i)
						start += (string)" " + words[i];
				} else {
					start = words[words.size() - markovOrder];
					for(unsigned i = 1; i < markovOrder; ++i)
						start += (string)" " + words[words.size() - markovOrder + i];
				}

				next = fetch(start);
				if(!next.empty())
					words.push_back(next);
			} while(!next.empty());

			stringstream chain;
			chain << words[0];
			for(unsigned i = 1; i < words.size(); ++i)
				chain << " " << words[i];
			return chain.str();
		}

		virtual string name() const {
			return "markov";
		}
		virtual string help() const {
			return "Returns a markov chain.";
		}
		virtual string regex() const {
			return "^!markov\\s+(.*)";
		}
}; // }}}

// list chain count {{{
class ChainCountFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			string init = fargs.matches[1];
			vector<string> words = split(init, " \t");

			string start = "";
			if(words.size() < markovOrder) {
				start = words[0];
				for(unsigned i = 1; i < words.size(); ++i)
					start += (string)" " + words[i];
			} else {
				start = words[words.size() - markovOrder];
				for(unsigned i = 1; i < markovOrder; ++i)
					start += (string)" " + words[words.size() - markovOrder + i];
			}

			map<string, unsigned> seedMap = markovModel[start];
			unsigned total = 0;
			for(auto i = seedMap.begin(); i != seedMap.end(); ++i)
				total += i->second;

			stringstream ss;
			ss << "Chains starting with: " << start << ": "
				<< markovModel[start].size() << " ["
				<< total << ", " << markovModel.size() << "]";

			return ss.str();
		}

		virtual string name() const {
			return "ccount";
		}
		virtual string help() const {
			return "Return number of markov chains";
		}
		virtual string regex() const {
			return "^!c+ount(\\s.*)?";
		}
}; // }}}

// TODO: markov, is, forget

// say yes {{{
class YesFunction : public Function {
	public:
		YesFunction(string nick) : m_nick(nick) {
		}

		virtual string run(FunctionArguments fargs) {
			return "yes";
		}

		virtual string name() const {
			return this->m_nick;
		}
		virtual string help() const {
			return "Say yes when my name is said";
		}
		virtual string regex() const {
			return (string)".*" + this->m_nick + ".*";
		}

	protected:
		string m_nick;
}; // }}}

// regex all the things! {{{
class ReplaceFunction : public Function {
	public:
		virtual string run(FunctionArguments fargs) {
			string m2 = fargs.matches[1], m4 = fargs.matches[2];

			for(auto i = lastLog.rbegin(); i != lastLog.rend(); ++i) {
				string str = *i;
				vector<string> words = split(str, " \t");
				if(contains(words, m2)) {
					stringstream ss;
					for(auto j = words.begin(); j != words.end(); ++j) {
						if(*j == m2)
							ss << m4;
						else
							ss << *j;
						if(j != words.end() - 1)
							ss << " ";
					}
					return ss.str();
				}
			}

			return "error: not matched";
		}

		virtual string name() const {
			return "replace";
		}
		virtual string help() const {
			return (string)"Give it two regex, it finds the last message that" +
				" matches the first and substitutes it with the second";
		}
		virtual string regex() const {
			return "^!s\\s+([^\\s]+)\\s+([^\\s]+)";
		}
}; // }}}

int main(int argc, char **argv) {
	const string logFileName = "PINKSERV_TWO.log", myNick = "PINKSERV_TWO",
			markovFileName = "PINKSERV_TWO.markov";
	const string privmsgRegexExp =
		"^:([A-Za-z0-9_]*)!([-/@~A-Za-z0-9_\\.]*) PRIVMSG ([#A-Za-z0-9_]*) :(.*)";
	const string joinRegexExp =
		"^:([A-Za-z0-9_]*)!([-/@~A-Za-z0-9_\\.]*) JOIN :([#A-Za-z0-9_]*)";
	const string toUsRegexExp = "^(" + myNick + "[L:\\,]?\\s+).*";
	const string toUsRRegexExp = "^(" + myNick + "[L:\\,]?\\s+)";
	const string helpRegexExp = "^\\s*help(\\s+(\\S+))?";

	split("  This    is a sequence     of \t words . ", " \t");

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
	moduleMap["list"] = new ListFunction();
	moduleMap["!s"] = new ReplaceFunction();

	moduleMap["markov"] = new MarkovFunction();
	moduleMap["ccount"] = new ChainCountFunction();
	moduleMap["yes"] = new YesFunction(myNick);

	regex privmsgRegex(privmsgRegexExp, regex::perl);
	regex joinRegex(privmsgRegexExp, regex::perl);
	regex toUsRegex(toUsRegexExp, regex::perl);
	regex toUsRRegex(toUsRRegexExp, regex::perl);
	regex helpRegex(helpRegexExp, regex::perl);

	ofstream log("pbrane.log");
	if(!log.good()) {
		cerr << "Could not open log!" << endl;
		return 1;
	}

	log << "pbrane started." << endl;

	ifstream in(markovFileName);
	if(in.good()) {
		log << "reading markov chain entries" << endl;
		unsigned lcount = 0;
		while(!in.eof()) {
			string line;
			getline(in, line);
			if(in.eof() || !in.good())
				break;
			insert(line);
			lcount++;
			if(lcount % 50000 == 0)
				log << "\t" << lcount << "..." << endl;
		}
		log << "\t" << markovFileName << ": read " << lcount << " lines" << endl;
	}

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
			//log << "pmsg: " << matches[0] << endl;
			//log << "  nick: " << matches[1] << endl;
			//log << "  user: " << matches[2] << endl;
			//log << "target: " << matches[3] << endl;
			//log << "   msg: " << matches[4] << endl;
			log << matches[1] << "@" << matches[3] << ": " << matches[4] << endl;
			string message(matches[4]), nick(matches[1]), omessage(matches[4]);

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
			} else if(toUs && regex_match(message, matches, helpRegex)) {
				string function = matches[2], res = "(null)";
				if(function.empty()) {
					stringstream ss;
					unsigned j = 0;
					for(auto i = moduleMap.begin(); i != moduleMap.end(); ++i, ++j) {
						ss << i->second->name();
						if(j != moduleMap.size() - 1)
							ss << ", ";
					}
					res = ss.str();
				} else {
					if(moduleMap.find(function) == moduleMap.end()) {
						bool isName = false;
						for(auto i = moduleMap.begin(); i != moduleMap.end(); ++i) {
							if(i->second->name() == function) {
								isName = true;
								res = i->second->help();
							}
						}
						if(!isName)
							res = "That function does not exist";
					} else {
						res = moduleMap[function]->help();
					}
				}
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

				bool matched = false;
				// loop through setup modules trying to match their regex
				for(auto mod = moduleMap.begin(); mod != moduleMap.end(); ++mod) {
					regex cmodr(mod->second->regex(), regex::perl);
					// if this module matches
					if(regex_match(message, fargs.matches, cmodr, match_extra)) {
						// log that we got a hit
						log << "module matched: " << mod->first << endl;
						// run the module
						string res = mod->second->run(fargs);
						if(res.empty()) {
							log << "module returned nothing, moving on" << endl;
						} else {
							// log the output/send the output
							log << " -> " << rtarget << " :" << res << endl;
							cout << "PRIVMSG " << rtarget << " :" << res << endl;
							matched = true;
							break;
						}
					}
				}

				if(!matched) {
					lastLog.push_back(omessage);
					regex yesCommand(moduleMap["yes"]->regex(), regex::perl);
					if(toUs || regex_match( message, fargs.matches, yesCommand, match_extra)) {
						log << " -> " << rtarget << " :yes" << endl;
						cout << "PRIVMSG " << rtarget << " :yes" << endl;
					}
					insert(message);
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

	// free memory associated with modules
	for(auto i = moduleMap.begin(); i != moduleMap.end(); ++i) {
		delete i->second;
	}

	return 0;
}

