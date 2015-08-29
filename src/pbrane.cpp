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

#include <fstream>
using std::ifstream;
using std::ofstream;

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
#include "expression.hpp"
#include "parser.hpp"
#include "events.hpp"
#include "regex.hpp"

void process(string network, string script, string nick, string target);
string evaluate(string script, string nick);

struct PrivateMessage {
	string network;
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
bool import = false;

struct CrashInformation {
	bool crashed{false};
	string last{};
};

CrashInformation crashed();
void crashed(bool val, string last = "");

void cycle_brain();
void prettyPrint(string arg);
void teval(vector<string> args);

void cycle_brain() {
	// initialize modules
	modules::init(config::brainFileName);

	// free memory associated with modules
	modules::deinit(config::brainFileName);
}

void prettyPrint(string arg) {
	cout << arg << endl;
	try {
		auto et = parse(arg);
		cout << et->pretty(' ', 4) << endl;
	} catch(ParseException e) {
		cerr << e.pretty() << endl;
	}
}

void teval(vector<string> args) {
	global::vars["bot.owner"] = "jac";
	global::vars["bot.admins"] = "Jext, RGCockatrices, bonzairob, quairlzr, Nybbles, ajanata";
	global::vars["bot.maxIterations"] = "10";

	// initialize modules
	modules::init(config::brainFileName);

	random_device randomDevice;
	unsigned int seed = randomDevice();
	global::init(seed);

	if(!args.empty()) {
		for(auto &arg : args) {
			if(arg.empty())
				continue;
			cout << ": " << arg << endl;

			try {
				auto expr = Parser::parse(arg);
				cout << "expr: " << (expr ? "true" : "false") << endl;

				// print computed AST
				cout << "final: " << endl;
				cout << expr->pretty() << endl;
				cout << "stringify: " << expr->toString() << endl;

				cout << "result: " << expr->evaluate("jac").toString() << endl;
				// TODO: other exception types...
			} catch(ParseException e) {
				cout << e.pretty() << endl;
			} catch(StackTrace e) {
				cout << e.toString() << endl;
			} catch(string &s) {
				cout << "\t: " << s << endl;
			}
		}

		// free memory associated with modules
		modules::deinit(config::brainFileName);
		return;
	}

	while(cin.good() && !cin.eof()) {
		string nick, line;
		getline(cin, nick);
		getline(cin, line);
		if(nick.empty() || line.empty())
			break;

		try {
			auto expr = Parser::parse(line);

			cerr << "eval'ing: " << line << " as " << nick << endl;
			cerr << "final AST: " << endl;
			cerr << expr->pretty() << endl;
			cerr << "stringify: " << expr->toString() << endl;

			string res = expr->evaluate(nick).toString();
			cerr << "result: " << res << endl;
			cout << nick + ": " << res << endl;

			// TODO: other exception types
		} catch(ParseException e) {
			cerr << e.pretty() << endl;
		} catch(StackTrace e) {
			cout << e.toString() << endl;
		} catch(string &s) {
			cerr << "\texception: " << s << endl;
			cout << nick + ": error: " + s << endl;
		}
	}

	// free memory associated with modules
	modules::deinit(config::brainFileName);
}

int main(int argc, char **argv) {
	unsigned int seed = 0;
	if(argc > 1) {
		string arg{argv[1]};
		if(arg == "--import") {
			import = true;
			cerr << "pbrane: import mode enabled" << endl;
		} else if(arg == "--teval") {
			vector<string> args;
			for(int i = 2; i < argc; ++i)
				args.push_back(argv[i]);
			teval(args);
			return 0;
		} else if(arg == "--cycle") {
			cycle_brain();
			return 0;
		} else if(arg == "--pprint") {
			for(int i = 2; i < argc; ++i)
				prettyPrint(argv[i]);
			return 0;
		} else
			seed = fromString<unsigned int>(argv[1]);
	} else {
		random_device randomDevice;
		seed = randomDevice();
	}

	if(!global::init(seed)) {
		cerr << "pbrane: global::init failed" << endl;
		return -1;
	}
	modules::init(config::brainFileName);

	// TODO: don't hard-code these. These should be set in the startup file?
	evaluate("${(!undefined 'bot.owner')? { bot.owner = 'jac'; }}", "jac");
	evaluate("${(!undefined 'bot.nick')? { bot.nick = 'PINKSERV3'; }}",
			global::vars["bot.owner"].toString());
	evaluate("${(!undefined 'bot.maxLineLength')? { bot.maxLineLength = 256; }}",
			global::vars["bot.owner"].toString());
	if(import) {
		evaluate("${!on \"text\" (null => !ngobserve text)}",
				global::vars["bot.owner"].toString());
	}

	if(!global::secondaryInit()) {
		cerr << "pbrane: global::secondaryInit failed" << endl;
		// TODO: this should fail out completely?
	}


	global::log << "----- " << global::vars["bot.nick"].toString() << " started -----" << endl;
	cerr << "----- " << global::vars["bot.nick"].toString() << " started -----" << endl;

	global::secondaryInit(); // TODO: we do this twice?
	journal::init();

	CrashInformation ci = crashed();
	if(ci.crashed) {
		cerr << "-- looks like I crashed" << endl;
		send("slashnet", "#jitro", "oh no, " +
				(ci.last.empty() ? "I crashed?" : ci.last + " made me crash?") +
				" :(", true);
	}

	crashed(true, "{startup}");

	// while there is more input coming
	global::done = false;
	while(!cin.eof() && !global::done) {
		// read the current line of input
		string line;
		getline(cin, line);

		if(line.find_first_not_of(" \t\r\n") == string::npos)
			continue;

		string network = line.substr(0, line.find(" "));
		line = line.substr(line.find(" ") + 1);

		if(line.find_first_not_of(" \t\r\n") == string::npos)
			continue;

		journal::Entry entry(line);

		vector<string> fields = split(line);
		if(fields[1] == (string)"PRIVMSG") {
			string nick = fields[0].substr(1, fields[0].find("!") - 1);
			crashed(true, nick);

			size_t mstart = line.find(":", 1);
			string message = line.substr(mstart + 1);

			string target = fields[2];
			if(fields[2] == global::vars["bot.nick"].toString())
				target = nick;

			// check for a special hook
			bool wasHook = false;
			for(auto h : hooks)
				if((*h)({ network, message, nick, target })) {
					wasHook = true;
					break;
				}

			// TODO: proper environment for triggers
			global::vars["nick"] = nick;
			global::vars["text"] = message;

			if(wasHook)
				entry.etype = journal::ExecuteType::Hook;
			// if the line is a ! command, run it
			else if(message[0] == '!' && message.length() > 1) {
				// it might be a !: to force intepretation line
				if(message.size() > 1 && message[1] == ':')
					process(network, message.substr(2), nick, target);
				else
					process(network, message, nick, target);
				entry.etype = journal::ExecuteType::Function;
			} else if(message.substr(0, 2) == (string)"${" && message.back() == '}') {
				process(network, message, nick, target);
				entry.etype = journal::ExecuteType::Function;
			}
			// otherwise, run on text triggers
			else {
				entry.etype = journal::ExecuteType::None;

				vector<Variable> results = EventSystem::process(EventType::Text);
				if(results.size() == 1)
					send(network, target, results.front().toString(), true);
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

			vector<Variable> results = EventSystem::process(EventType::Join);
			if(results.size() == 1)
				send(network, where, results.front().toString(), true);
		}
		if(fields[1] == (string)"NICK") {
			;// run nick triggers
		}
		if((fields[1] == (string)"PART") || (fields[1] == (string)"QUIT")) {
			;// run leave triggers
		}

		journal::push(entry);
		global::log << entry.format() << endl;
	}

	cerr << "pbrane: exited main loop" << endl;
	journal::deinit();

	// free memory associated with modules
	modules::deinit(config::brainFileName);

	// deinit global
	global::deinit();

	crashed(false, "{shutdown?}");

	return 0;
}

void process(string network, string script, string nick, string target) {
	if(import)
		return;
	script = trim(script);
	if(script.empty())
		return;

	bool plainFunction = false;
	string plainFName;
	if(script[0] == '!')
		plainFunction = true, plainFName = script.substr(1, script.find(" ") - 1);
	string noF = plainFName + " does not exist as a callable function [stacktrace: !]";
	string result = evaluate(script, nick);
	if(plainFunction && result == noF) {
		cerr << "simple call to nonexistante function error supressed" << endl;
		return;
	}
	// assume we can run the script
	send(network, target, result, true);
}
string evaluate(string script, string nick) {
	try {
		cerr << "evaluate: " << script << endl;
		auto expr = Parser::parse(script);
		if(!expr)
			cerr << "expr is null" << endl;
		string res = expr->evaluate(nick).toString();
		cerr << "res: " << res << endl;
		return res;
	} catch(ParseException e) {
		cerr << e.pretty() << endl;
		return e.msg + " @" + asString(e.idx);
	} catch(StackTrace e) {
		cerr << e.toString() << endl;
		return e.toString();
	} catch(string &s) {
		cerr << "string type error: " << s << endl;
		return s;
	}
}


bool powerHook(PrivateMessage pmsg) {
	if(import)
		return false;
	if(pmsg.message[0] == ':') // ignore starting colon
		pmsg.message = pmsg.message.substr(1);
	if(pmsg.message == (string)"!restart" && isOwner(pmsg.nick))
		return global::done = true;
	if(pmsg.message == (string)"!save")
		return global::done = true;
	return false;
}
bool regexHook(PrivateMessage pmsg) {
	if(import)
		return false;
	if(pmsg.message[0] != 's')
		return false;
	if(pmsg.message.size() < 2)
		return false;
	if(((string)":/|").find(pmsg.message[1]) == string::npos)
		return false;

	try {
		Regex r(pmsg.message.substr(1));
		auto entries = journal::search(r.match());
		if(entries.empty())
			return true;
		for(auto it = entries.rbegin(); it != entries.rend(); ++it) {
			// only replace on non-executed things
			if(it->etype == journal::ExecuteType::Hook || it->etype == journal::ExecuteType::Function)
				continue;
			// if a nick was specified as a flag and it's not who said it, continue
			if(!r.flags().empty() && r.flags() != it->nick())
				continue;
			string result;
			r.execute(it->arguments, result);
			if(it->etype == journal::ExecuteType::None)
				send(pmsg.network, pmsg.target, "<" + it->nick() + "> " + result, true);
			else
				send(pmsg.network, pmsg.target, result, true);
			break;
		}
		return true;
	} catch(string e) {
		send(pmsg.network, pmsg.target, e, true);
	}

	return false;
}

CrashInformation crashed() {
	CrashInformation ci;
	ifstream in("PINKSERV3.crashed");
	if(!in.good()) {
		cerr << "crashed file does not exist" << endl;
		return ci;
	}
	string l;
	getline(in, l);
	ci.crashed = (l == "crashed");
	getline(in, ci.last);
	return ci;
}
void crashed(bool val, string last) {
	ofstream out("PINKSERV3.crashed");
	if(val)
		out << "crashed" << endl;
	else
		out << "safe" << endl;
	out << last << endl;
}

