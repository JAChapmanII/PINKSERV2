#include "bot.hpp"
using std::string;
using std::vector;
using zidcu::Database;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <map>
using std::map;

#include "util.hpp"
#include "parser.hpp"
#include "expression.hpp"


void sqlTrace(void *a, const char *b);
string escapeForPMSG(string str);
bool allSpaces(string str);

void sqlTrace(void *, const char *b) { cerr << "sqlTrace: " << b << endl; }

string escapeForPMSG(string str) {
	string res;
	for(unsigned i = 0; i < str.length(); ++i)
		switch(str[i]) {
			case '\n':
				if(res.length() > 0 && res.back() == ' ')
					continue;
				res += ' ';
				break;
			default:
				res += str[i];
				continue;
				break;
		}
	return util::trim(res);
}
bool allSpaces(string str) {
	for(auto c : str)
		if(!isspace(c))
			return false;
	return true;
}


Bot::Bot(Database &idb, Options iopts, Clock iclock, ExtraSetup setup) :
		db{idb}, opts{iopts}, clock{iclock}, journal{db},
		events{db, opts.debugEventSystem}, dictionary{db}, vars{db}, vm{vars},
		ngStore{db}, todos{journal, db}, rengine{} {
	rengine.seed(opts.seed);

	db.executeVoid("PRAGMA cache_size = 10000;");
	db.executeVoid("PRAGMA page_size = 8192;");
	db.executeVoid("PRAGMA temp_store = MEMORY;");
	db.executeScalar<string>("PRAGMA journal_mode = WAL;");
	db.executeVoid("PRAGMA synchronous = NORMAL;");

	if(opts.debugSQL)
		void* res = sqlite3_trace(db.getDB(), sqlTrace, nullptr);

	setup(this);
	events.process(EventType::BotStartup, vm);

	// TODO: local variables?
	// TODO: variable permissions?
}
Bot::~Bot() { events.process(EventType::BotShutdown, vm); }

void Bot::send(string network, string target, string line, bool send) {
	if(!send) return;

	line = escapeForPMSG(line); // TODO

	auto maxLineLength = util::fromString<unsigned>(
			vars.get("bot.maxLineLength").toString());
	if(line.length() > maxLineLength)
		line = line.substr(0, maxLineLength);

	if(allSpaces(line))
		return;

	cout << network << " PRIVMSG " << target << " :" << line << endl;

	auto us = vars.get("bot.nick").toString() + "!~self@localhost";
	Entry e{-1, clock.now(), SentType::Sent, ExecuteType::Sent, network,
		":" + us + " PRIVMSG " + target + " :" + line};
	journal.upsert(e);
}

bool Bot::isOwner(std::string nick) {
	return (nick == vars.get("bot.owner").toString());
}
bool Bot::isAdmin(std::string nick) {
	return util::contains(vars.get("bot.admins").toString(), " " + nick + " ");
}

// TODO: eliminate
string Bot::evaluate(string script, string nick) {
	try {
		auto expr = Parser::parse(script);
		if(!expr) {
			cerr << "Bot::evaluate: \"" << script << "\" is null expr" << endl;
			return "";
		}
		auto res = expr->evaluate(vm, nick).toString();
		return res;
	} catch(ParseException e) {
		cerr << e.pretty() << endl;
		return e.msg + " @" + util::asString(e.idx);
	} catch(StackTrace e) {
		cerr << e.toString() << endl;
		return e.toString();
	} catch(string &s) {
		cerr << "string type error: " << s << endl;
		return s;
	}
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

void Bot::process(string network, string script, string nick, string target) {
	if(opts.import)
		return;

	script = util::trim(script);
	if(script.empty())
		return;

	static map<Context, string> contextMap;

	if(script.substr(0, 2) == "::") {
		auto context = Context{ network, nick, target };
		script = util::trim(script.substr(2));
		if(script.back() == '\\') {
			script.pop_back();
			contextMap[context] += script + " ";
			return;
		}

		script = "${ " + contextMap[context] + script + " }";
		contextMap[context] = "";
	}

	bool simpleCall{false};
	string result = "";
	try {
		auto expr = Parser::parse(script);
		if(!expr) {
			cerr << "Bot::evaluate: \"" << script << "\" is null expr" << endl;
		} else {
			if(expr->type == "!") simpleCall = true;
			result = expr->evaluate(vm, nick).toString();
		}
	} catch(ParseException e) {
		cerr << e.pretty() << endl;
		auto lines = util::split(e.pretty(), "\n");
		if(lines.size() != 3) {
			result = e.msg + " @" + util::asString(e.idx);
		} else {
			// TODO: jank
			this->send(network, target, ":" + lines[1].substr(3), true);
			this->send(network, target, ":" + lines[2].substr(3) + "  " + lines[0], true);
			return;
		}
	} catch(StackTrace e) {
		cerr << e.toString() << endl;
		if(e.type == ExceptionType::FunctionDoesNotExist && simpleCall) {
			cerr << "simple call to nonexistant function error supressed" << endl;
		} else {
			result = e.toString();
		}
	} catch(string &s) {
		cerr << "string type error: " << s << endl;
		result = s;
	}

	// assume we can run the script
	this->send(network, target, result, true);
}

