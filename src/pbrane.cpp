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

bool import = false;

void prettyPrint(string arg);
void teval(vector<string> args);

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
	global::vars.set("bot.owner", "jac");
	global::vars.set("bot.admins", "jac");

	// initialize modules
	modules::init();

	random_device randomDevice;
	unsigned int seed = randomDevice();
	global::init(seed);

	if(!args.empty()) {
		for(auto &arg : args) {
			if(arg.empty() || startsWith(arg, "--"))
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
		modules::deinit();
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
	modules::deinit();
}

int main(int argc, char **argv) {
	vector<string> args;
	for(int i = 1; i < argc; ++i)
		args.push_back(argv[i]);

	unsigned int seed = 0;
	for(auto &arg : args) {
		if (arg == "--teval") {
			teval(args);
			return 0;
		}
		if(arg == "--pprint") {
			for(auto &arg2 : args)
				if(!startsWith(arg2, "--"))
					prettyPrint(arg2);
			return 0;
		}

		if(arg == "--import") {
			import = true;
			cerr << "pbrane: import mode enabled" << endl;
		} else if(arg == "--debugSQL") {
			global::debugSQL = true;
			cerr << "pbrane: debug sql enabled" << endl;
		} else if(arg == "--debugEventSystem") {
			global::debugEventSystem = true;
			cerr << "pbrane: debug event system enabled" << endl;
		} else if(arg == "--debugFunctionBody") {
			global::debugFunctionBody = true;
			cerr << "pbrane: debug function body enabled" << endl;
		} else {
			seed = fromString<unsigned int>(argv[1]);
		}
	}

	if(args.empty()) {
		random_device randomDevice;
		seed = randomDevice();
	}

	if(!global::init(seed)) {
		cerr << "pbrane: global::init failed" << endl;
		return -1;
	}
	modules::init();

	// TODO: don't hard-code these. These should be set in the startup file?
	evaluate("${(!undefined 'bot.owner')? { bot.owner = 'jac'; }}", "jac");
	evaluate("${(!undefined 'bot.nick')? { bot.nick = 'PINKSERV3'; }}",
			global::vars.getString("bot.owner"));
	evaluate("${(!undefined 'bot.maxLineLength')? { bot.maxLineLength = 256; }}",
			global::vars.getString("bot.owner"));
	if(import) {
		evaluate("${!on \"text\" (null => !ngobserve text)}",
				global::vars.getString("bot.owner"));
	}

	if(!global::secondaryInit()) {
		cerr << "pbrane: global::secondaryInit failed" << endl;
		// TODO: this should fail out completely?
	}


	global::log << "----- " << global::vars.getString("bot.nick") << " started -----" << endl;
	cerr << "----- " << global::vars.getString("bot.nick") << " started -----" << endl;

	global::secondaryInit(); // TODO: we do this twice?

	if(global::vars.defined("bot.crashed")) {
		cerr << "-- looks like I crashed" << endl;
		send("slashnet", "#jitro", "oh no, '"
				+ global::vars.getString("bot.crashed") + "' made me crash?", true);
	}
	global::vars.erase("bot.crashed");

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

		Entry entry{global::now(), network, line};
		global::journal.upsert(entry);

		vector<string> fields = split(line);
		if(fields[1] == (string)"PRIVMSG") {
			string nick = fields[0].substr(1, fields[0].find("!") - 1);
			global::vars.set("bot.crashed", nick);

			size_t mstart = line.find(":", 1);
			string message = line.substr(mstart + 1);

			string target = fields[2];
			if(fields[2] == global::vars.getString("bot.nick"))
				target = nick;

			// check for a special hook
			bool wasHook = false;
			for(auto h : hooks)
				if((*h)({ network, message, nick, target })) {
					wasHook = true;
					break;
				}

			// TODO: proper environment for triggers
			global::vars.set("nick", nick);
			global::vars.set("text", message);

			if(wasHook)
				entry.etype = ExecuteType::Hook;
			// if the line is a ! command, run it
			else if(message[0] == '!' && message.length() > 1) {
				// it might be a !: to force intepretation line
				if(message.size() > 1 && message[1] == ':')
					process(network, message.substr(2), nick, target);
				else
					process(network, message, nick, target);
				entry.etype = ExecuteType::Function;
			} else if(message.substr(0, 2) == (string)"${" && message.back() == '}') {
				process(network, message, nick, target);
				entry.etype = ExecuteType::Function;
			}
			// otherwise, run on text triggers
			else {
				entry.etype = ExecuteType::None;

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
			global::vars.set("nick", nick);
			global::vars.set("where", where);

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

		global::journal.upsert(entry);
	}

	cerr << "pbrane: exited main loop" << endl;
	global::vars.set("bot.crashed", "{shutdown}");

	// free memory associated with modules
	modules::deinit();

	// deinit global
	global::deinit();

	global::vars.erase("bot.crashed");

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
		auto entries = global::journal.fetch(AndPredicate{
			[=](Entry &e) {
				// only replace on non-executed things
				if(e.etype == ExecuteType::Hook || e.etype == ExecuteType::Function
						|| e.etype == ExecuteType::Unknown)
					return false;
				// if a nick was specified as a flag and it's not who said it, skip
				if(!r.flags().empty() && r.flags() != e.nick())
					return false;
				return true;
			}, RegexPredicate{r.match()}}, 1);

		if(entries.empty())
			return true;
		auto e = entries.front();

		string result;
		r.execute(e.arguments, result);
		if(e.etype == ExecuteType::None)
			send(pmsg.network, pmsg.target, "<" + e.nick() + "> " + result, true);
		else
			send(pmsg.network, pmsg.target, result, true);

		return true;
	} catch(string e) {
		send(pmsg.network, pmsg.target, e, true);
	}

	return false;
}

