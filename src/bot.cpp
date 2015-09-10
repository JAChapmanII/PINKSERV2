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


Bot::Bot(Database &db, Options opts, Clock clock) : _db{db}, _opts{opts},
		_clock{clock}, _journal{_db}, _events{_db, opts.debugEventSystem},
		_dictionary{_db}, _vars{_db}, _vm{_vars}, _rengine{} {
	_rengine.seed(_opts.seed);

	_db.executeVoid("PRAGMA cache_size = 10000;");
	_db.executeVoid("PRAGMA page_size = 8192;");
	_db.executeVoid("PRAGMA temp_store = MEMORY;");
	_db.executeScalar<string>("PRAGMA journal_mode = WAL;");
	_db.executeVoid("PRAGMA synchronous = NORMAL;");

	if(_opts.debugSQL)
		void* res = sqlite3_trace(db.getDB(), sqlTrace, nullptr);

	// TODO: these
	// variable, function map
	// local variable map
	// variable permission map
}



bool Bot::done() const { return _done; }
void Bot::done(bool ndone) { _done = ndone; }

bool Bot::secondaryInit(string startupFile) {
	ifstream startup(startupFile);
	if(startup.good()) {
		_journal.log(_clock.now(), "reading startup file: " + startupFile);

		string line;
		while(startup.good() && !startup.eof()) {
			// TODO: make fake logitem?
			getline(startup, line);
			if(startup.eof())
				break;
			if(line.empty())
				continue;

			_journal.log(_clock.now(), "startup line: " + line);
			try {
				auto expr = Parser::parse(line);
				string result = expr->evaluate(_vm,
						_vars.getString("bot.owner")).toString();
			// TODO: more exception types
			} catch(ParseException e) {
				_journal.log(_clock.now(), "parse exception: " + e.pretty());
			} catch(StackTrace e) {
				_journal.log(_clock.now(), "stack trace exception: " + e.toString());
			} catch(string &e) {
				_journal.log(_clock.now(), "string exception: " + e);
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
			_vars.getString("bot.maxLineLength"));
	if(line.length() > maxLineLength)
		line = line.substr(0, maxLineLength);

	if(allSpaces(line))
		return;

	cout << network << " PRIVMSG " << target << " :" << line << endl;

	auto us = _vars.get("bot.nick").toString() + "!~self@localhost";
	Entry e{-1, _clock.now(), SentType::Sent, ExecuteType::Sent, network,
		":" + us + " PRIVMSG " + target + " :" + line};
	_journal.upsert(e);
}

bool Bot::isOwner(std::string nick) {
	return (nick == _vars.getString("bot.owner"));
}
bool Bot::isAdmin(std::string nick) {
	return util::contains(_vars.getString("bot.admins"), " " + nick + " ");
}

string Bot::set(string name, string val) {
	return _vars.set(name, val).toString();
}
string Bot::get(string name) {
	return _vars.getString(name);
}
bool Bot::defined(string name) {
	return _vars.defined(name);
}
void Bot::erase(string name) {
	_vars.erase(name);
}

sqlite_int64 Bot::upsert(Entry &entry) {
	return _journal.upsert(entry);
}
vector<Entry> Bot::fetch(EntryPredicate predicate, int limit) {
	return _journal.fetch(predicate, limit);
}

vector<Variable> Bot::process(EventType etype) {
	return _events.process(etype, _vm);
}

Pvm &Bot::vm() { return _vm; }

