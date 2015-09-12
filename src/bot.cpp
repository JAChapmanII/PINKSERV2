#include "bot.hpp"
using std::string;
using std::vector;
using zidcu::Database;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <fstream>
using std::ifstream;

#include <ctime>

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



sqlite_int64 Clock::now() {
	// TODO: fix
	return time(NULL);
}

Bot::Bot(Database &db, Options opts, Clock clock) : db{db}, opts{opts},
		clock{clock}, journal{db}, events{db, opts.debugEventSystem},
		dictionary{db}, vars{db}, vm{vars}, ngStore{db}, rengine{} {
	rengine.seed(opts.seed);

	db.executeVoid("PRAGMA cache_size = 10000;");
	db.executeVoid("PRAGMA page_size = 8192;");
	db.executeVoid("PRAGMA temp_store = MEMORY;");
	db.executeScalar<string>("PRAGMA journal_mode = WAL;");
	db.executeVoid("PRAGMA synchronous = NORMAL;");

	if(opts.debugSQL)
		void* res = sqlite3_trace(db.getDB(), sqlTrace, nullptr);

	// TODO: these
	// variable, function map
	// local variable map
	// variable permission map
}


bool Bot::secondaryInit(string startupFile) {
	ifstream startup(startupFile);
	if(startup.good()) {
		journal.log(clock.now(), "reading startup file: " + startupFile);

		string line;
		while(startup.good() && !startup.eof()) {
			// TODO: make fake logitem?
			getline(startup, line);
			if(startup.eof())
				break;
			if(line.empty())
				continue;

			journal.log(clock.now(), "startup line: " + line);
			try {
				auto expr = Parser::parse(line);
				string result = expr->evaluate(vm,
						vars.getString("bot.owner")).toString();
			// TODO: more exception types
			} catch(ParseException e) {
				journal.log(clock.now(), "parse exception: " + e.pretty());
			} catch(StackTrace e) {
				journal.log(clock.now(), "stack trace exception: " + e.toString());
			} catch(string &e) {
				journal.log(clock.now(), "string exception: " + e);
			}
		}
		return true;
	}
	return false;
}

void Bot::send(string network, string target, string line, bool send) {
	if(!send) return;

	line = escapeForPMSG(line); // TODO

	auto maxLineLength = util::fromString<unsigned>(
			vars.getString("bot.maxLineLength"));
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
	return (nick == vars.getString("bot.owner"));
}
bool Bot::isAdmin(std::string nick) {
	return util::contains(vars.getString("bot.admins"), " " + nick + " ");
}

