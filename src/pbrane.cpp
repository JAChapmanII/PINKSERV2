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

void process(Bot &bot, string network, string script, string nick, string target);
string evaluate(Bot &bot, string script, string nick);

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

	// TODO: don't hard-code these. These should be set in the startup file?
	if(import) {
		evaluate(pbrane, "${!on \"text\" (null => !ngobserve text)}",
				pbrane.vars.getString("bot.owner"));
	}


	// while there is more input coming
	while(!cin.eof() && !pbrane.done) {
		// read the current line of input
		string line;
		getline(cin, line);

		if(line.find_first_not_of(" \t\r\n") == string::npos)
			continue;

		string network = line.substr(0, line.find(" "));
		line = line.substr(line.find(" ") + 1);

		if(line.find_first_not_of(" \t\r\n") == string::npos)
			continue;

		Entry entry{Clock{}.now(), network, line};
		pbrane.journal.upsert(entry);

		auto fields = split(line);
		if(fields.empty()) continue;

		if(fields[1] == (string)"PRIVMSG") {
			string nick = fields[0].substr(1, fields[0].find("!") - 1);

			size_t mstart = line.find(":", 1);
			string message = line.substr(mstart + 1);

			string target = fields[2];
			if(fields[2] == pbrane.vars.getString("bot.nick"))
				target = nick;

			// check for a special hook
			bool wasHook = false;
			for(auto h : hooks)
				if((*h)({ network, message, nick, target, pbrane })) {
					wasHook = true;
					break;
				}

			// TODO: proper environment for triggers
			pbrane.vars.set("nick", nick);
			pbrane.vars.set("text", message);

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
		if(fields[1] == (string)"JOIN") {
			string nick = fields[0].substr(1, fields[0].find("!") - 1);
			string where = fields[2];
			if(where[0] == ':')
				where = where.substr(1);

			// TODO: proper environment for triggers
			pbrane.vars.set("nick", nick);
			pbrane.vars.set("where", where);

			vector<Variable> results = pbrane.events.process(EventType::Join, pbrane.vm);
			if(results.size() == 1)
				pbrane.send(network, where, results.front().toString(), true);
		}
		if(fields[1] == (string)"NICK") {
			;// run nick triggers
		}
		if((fields[1] == (string)"PART") || (fields[1] == (string)"QUIT")) {
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

	string result = evaluate(bot, script, nick);
	if(plainFunction && result == noF) {
		cerr << "simple call to nonexistant function error supressed" << endl;
		return;
	}
	// assume we can run the script
	bot.send(network, target, result, true);
}
string evaluate(Bot &bot, string script, string nick) {
	try {
		cerr << "evaluate: " << script << endl;
		auto expr = Parser::parse(script);
		if(!expr)
			cerr << "expr is null" << endl;
		string res = expr->evaluate(bot.vm, nick).toString();
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

	try {
		Regex r(pmsg.message.substr(1));
		auto entries = pmsg.bot.journal.fetch(AndPredicate{
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
			pmsg.bot.send(pmsg.network, pmsg.target, "<" + e.nick() + "> " + result, true);
		else
			pmsg.bot.send(pmsg.network, pmsg.target, result, true);

		return true;
	} catch(string e) {
		pmsg.bot.send(pmsg.network, pmsg.target, e, true);
	}

	return false;
}

