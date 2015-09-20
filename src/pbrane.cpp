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

#include <map>
using std::map;

#include "global.hpp"
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
#include "eventsystem.hpp"
#include "expression.hpp"
#include "parser.hpp"
#include "regex.hpp"
#include "db.hpp"
using zidcu::Database;
#include "bot.hpp"
#include "sed.hpp"

void process(Bot &bot, string network, string script, string nick, string target);

struct PrivateMessage {
	string network{};
	string message{};
	string nick{};
	string target{};
	Bot &bot;
};
typedef bool (*hook)(PrivateMessage pmsg);

bool powerHook(PrivateMessage pmsg);
bool regexHook(PrivateMessage pmsg);

vector<hook> hooks = { &powerHook, &regexHook };

bool import = false, importLog{false};

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
	random_device randomDevice;
	auto seed = randomDevice();

	Database db{config::databaseFileName};
	Options opts{};
	opts.seed = seed;
	Bot pbrane{db, opts, Clock{}, modules::init};

	pbrane.vars.set("bot.owner", "jac");
	pbrane.vars.set("bot.admins", "jac");

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

				cout << "result: " << expr->evaluate(pbrane.vm, "jac").toString() << endl;
				// TODO: other exception types...
			} catch(ParseException e) {
				cout << e.pretty() << endl;
			} catch(StackTrace e) {
				cout << e.toString() << endl;
			} catch(string &s) {
				cout << "\t: " << s << endl;
			}
		}
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

			string res = expr->evaluate(pbrane.vm, nick).toString();
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
}

int main(int argc, char **argv) {
	vector<string> args;
	for(int i = 1; i < argc; ++i)
		args.push_back(argv[i]);

	Options opts{};
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
			opts.debugSQL = true;
			cerr << "pbrane: debug sql enabled" << endl;
		} else if(arg == "--debugEventSystem") {
			opts.debugEventSystem = true;
			cerr << "pbrane: debug event system enabled" << endl;
		} else if(arg == "--debugFunctionBodies") {
			opts.debugFunctionBodies = true;
			cerr << "pbrane: debug function bodies enabled" << endl;
		} else if(arg == "--importLog") {
			import = true;
			importLog = true;
			cerr << "pbrane: import log enabled" << endl;
		} else {
			opts.seed = fromString<unsigned int>(argv[1]);
		}
	}

	if(args.empty()) {
		random_device randomDevice;
		opts.seed = randomDevice();
	}

	Database db{config::databaseFileName};
	Bot pbrane{db, opts, Clock{}, modules::init};

	// while there is more input coming
	while(!cin.eof() && !pbrane.done) {
		// read the current line of input
		string line;
		getline(cin, line);

		if(line.find_first_not_of(" \t\r\n") == string::npos)
			continue;

		string network;
		auto ts = Clock{}.now();

		if(!importLog) {
			network = line.substr(0, line.find(" "));
			line = line.substr(line.find(" ") + 1);

			if(line.find_first_not_of(" \t\r\n") == string::npos)
				continue;
		} else {
			auto fs = util::split(line, "|");
			if(fs.size() < 3) {
				cerr << "unable to parse log line" << endl;
				cerr << "line: " << line << endl;
				return 32;
			}

			ts = util::fromString<sqlite_int64>(fs[0]);
			network = fs[1];
			line = util::join(fs.begin() + 2, fs.end(), " ");
		}

		Entry entry{ts, network, line};
		pbrane.journal.upsert(entry);

		auto fields = split(line);
		if(fields.empty()) continue;

		if(entry.type == EntryType::Text) {
			auto nick = entry.nick();
			auto message = entry.arguments;
			auto target = entry.where;

			if(target == pbrane.vars.getString("bot.nick"))
				target = nick;

			// TODO: proper environment for triggers
			pbrane.vars.set("nick", nick);
			pbrane.vars.set("text", message);

			// check for a special hook
			bool wasHook = false;
			for(auto h : hooks)
				if((*h)({ network, message, nick, target, pbrane })) {
					wasHook = true;
					break;
				}

			if(wasHook)
				entry.etype = ExecuteType::Hook;
			// if the line is a ! command, run it
			else if(message[0] == '!' && message.length() > 1) {
				// it might be a !: to force intepretation line
				if(message.size() > 1 && message[1] == ':')
					process(pbrane, network, message.substr(2), nick, target);
				else
					process(pbrane, network, message, nick, target);
				entry.etype = ExecuteType::Function;
			} else if(message.substr(0, 2) == (string)"${" && message.back() == '}') {
				process(pbrane, network, message, nick, target);
				entry.etype = ExecuteType::Function;
			} else if(message.substr(0, 2) == (string)"::") {
				process(pbrane, network, message, nick, target);
				entry.etype = ExecuteType::Function;
			}
			// otherwise, run on text triggers
			else {
				entry.etype = ExecuteType::None;

				vector<Variable> results = pbrane.events.process(EventType::Text, pbrane.vm);
				if(results.size() == 1)
					pbrane.send(network, target, results.front().toString(), true);
			}
		}
		if(entry.type == EntryType::Join) {
			auto nick = entry.nick(), where = entry.where;

			// TODO: proper environment for triggers
			pbrane.vars.set("nick", nick);
			pbrane.vars.set("where", where);

			auto results = pbrane.events.process(EventType::Join, pbrane.vm);
			if(results.size() == 1)
				pbrane.send(network, where, results.front().toString(), true);
		}
		if(entry.type == EntryType::Nick) {
			;// run nick triggers
		}
		if(entry.type == EntryType::Part || entry.type == EntryType::Quit) {
			;// run leave triggers
		}

		pbrane.journal.upsert(entry);
	}

	cerr << "pbrane: exited main loop" << endl;
	return 0;
}

struct Context {
	string network;
	string nick;
	string target;

	decltype(auto) toTuple() const {
		return make_tuple(network, nick, target);
	}
	bool operator<(const Context &rhs) const {
		return toTuple() < rhs.toTuple();
	}
};

void process(Bot &bot, string network, string script, string nick, string target) {
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

	static map<Context, string> contextMap;

	if(script.substr(0, 2) == "::") {
		auto context = Context{ network, nick, target };
		script = trim(script.substr(2));
		if(script.back() == '\\') {
			script.pop_back();
			contextMap[context] += script;
			return;
		}

		script = "${ " + contextMap[context] + " " + script + " }";
		contextMap[context] = "";
	}

	string result = bot.evaluate(script, nick);
	if(plainFunction && result == noF) {
		cerr << "simple call to nonexistant function error supressed" << endl;
		return;
	}
	// assume we can run the script
	bot.send(network, target, result, true);
}

bool powerHook(PrivateMessage pmsg) {
	if(import)
		return false;
	if(pmsg.message[0] == ':') // ignore starting colon
		pmsg.message = pmsg.message.substr(1);
	if(pmsg.message == (string)"!restart" && pmsg.bot.isOwner(pmsg.nick))
		return pmsg.bot.done = true;
	if(pmsg.message == (string)"!save")
		return pmsg.bot.done = true;
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

	auto regex = pmsg.message.substr(1);
	pmsg.bot.send(pmsg.network, pmsg.target, s(&pmsg.bot, regex), true);

	return true;
}

