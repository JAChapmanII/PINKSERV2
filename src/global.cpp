#include "global.hpp"
using std::string;
using std::to_string;
using std::vector;
using std::map;
using std::mt19937_64;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <fstream>
using std::ifstream;

#include <ctime>

#include "config.hpp"
#include "util.hpp"
using util::contains;
using util::fromString;
#include "expression.hpp"
#include "parser.hpp"

bool global::done;
mt19937_64 global::rengine;
vector<string> global::moduleFunctionList;

zidcu::Database global::db;
Journal global::journal{global::db};

VarStore global::vars{global::db, "vars", "var_perms" };
map<string, map<string, Variable>> global::lvars;

static unsigned int global_seed = 0;

vector<string> global::ignoreList;
unsigned global::minSpeakTime = 0;

void sqlTrace(void *a, const char *b);
void sqlTrace(void *, const char *b) { cerr << "sqlTrace: " << b << endl; }

bool global::debugSQL{false};
bool global::debugEventSystem{false};
bool global::debugFunctionBody{false};

Dictionary global::dictionary{global::db};

bool global::init(unsigned int seed) {
	// handle seeding the random number engine
	global_seed = seed;
	rengine.seed(seed);

	db.open(config::databaseFileName);

	db.executeVoid("PRAGMA cache_size = 10000;");
	db.executeVoid("PRAGMA page_size = 8192;");
	db.executeVoid("PRAGMA temp_store = MEMORY;");
	db.executeScalar<string>("PRAGMA journal_mode = WAL;");
	db.executeVoid("PRAGMA synchronous = NORMAL;");

	if(global::debugSQL)
		void* res = sqlite3_trace(db.getDB(), sqlTrace, nullptr);

	// TODO: these
	// variable, function map
	// local variable map
	// variable permission map

	return true;
}

bool global::secondaryInit() {
	ifstream startup(config::startupFile);
	if(startup.good()) {
		journal.log(now(), "reading startup file: " + config::startupFile);

		string line;
		while(startup.good() && !startup.eof()) {
			// TODO: make fake logitem?
			getline(startup, line);
			if(startup.eof())
				break;
			if(line.empty())
				continue;

			journal.log(now(), "startup line: " + line);
			try {
				auto expr = Parser::parse(line);
				string result = expr->evaluate(
						global::vars.getString("bot.owner")).toString();
			// TODO: more exception types
			} catch(ParseException e) {
				journal.log(now(), "parse exception: " + e.pretty());
			} catch(StackTrace e) {
				journal.log(now(), "stack trace exception: " + e.toString());
			} catch(string &e) {
				journal.log(now(), "string exception: " + e);
			}
		}
		return true;
	}
	return false;
}

string escapeForPMSG(string str);
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
bool allSpaces(string str);
bool allSpaces(string str) {
	for(auto c : str)
		if(!isspace(c))
			return false;
	return true;
}

void global::send(string network, string target, string line, bool send) {
	if(!send) return;

	line = escapeForPMSG(line); // TODO

	unsigned maxLineLength =
		fromString<unsigned>(global::vars.getString("bot.maxLineLength"));
	if(line.length() > maxLineLength)
		line = line.substr(0, maxLineLength);

	if(allSpaces(line))
		return;

	cout << network << " PRIVMSG " << target << " :" << line << endl;

	auto us = global::vars.get("bot.nick").toString() + "!~self@localhost";
	Entry e{-1, global::now(), SentType::Sent, ExecuteType::Sent, network,
		":" + us + " PRIVMSG " + target + " :" + line};
	global::journal.upsert(e);
}

bool global::isOwner(std::string nick) {
	return (nick == global::vars.getString("bot.owner"));
}
bool global::isAdmin(std::string nick) {
	return contains(global::vars.getString("bot.admins"), " " + nick + " ");
}

long long global::now() {
	// TODO: fix
	return time(NULL);
}

