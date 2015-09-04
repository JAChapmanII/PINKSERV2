#include "global.hpp"
using std::ofstream;
using std::ifstream;
using std::string;
using std::vector;
using std::fstream;
using std::map;
using std::mt19937_64;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <boost/regex.hpp>
using boost::regex;
using boost::smatch;
using boost::regex_match;
using boost::match_extra;

#include <ctime>

#include "config.hpp"
#include "modules.hpp"
#include "util.hpp"
using util::contains;
using util::split;
using util::fromString;
#include "journal.hpp"
#include "expression.hpp"
#include "parser.hpp"

bool global::done;
ofstream global::log;
mt19937_64 global::rengine;
vector<string> global::moduleFunctionList;

zidcu::Database global::db;
static bool db_setup{false};

VarStore global::vars{global::db, "vars", "var_perms" };
map<string, map<string, Variable>> global::lvars;

static unsigned int global_seed = 0;

vector<string> global::ignoreList;
unsigned global::minSpeakTime = 0;

void sqlTrace(void *a, const char *b);
void sqlTrace(void *a, const char *b) { cerr << "sqlTrace: " << b << endl; }

bool global::debugSQL{false};
bool global::debugEventSystem{false};
bool global::debugFunctionBody{false};

Dictionary global::dictionary{global::db};

bool global::init(unsigned int seed) {
	// handle seeding the random number engine
	global_seed = seed;
	rengine.seed(seed);

	log.open(config::logFileName, fstream::app);
	if(!log.good()) {
		cerr << "global::init: could not open log file!" << endl;
		return false;
	}

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
		log << "reading startup file: " << config::startupFile << endl;
		string line;
		while(startup.good() && !startup.eof()) {
			// TODO: make fake logitem?
			getline(startup, line);
			if(startup.eof())
				break;
			if(line.empty())
				continue;
			log << "\t" << line << endl;
			try {
				auto expr = Parser::parse(line);
				string result = expr->evaluate(
						global::vars.getString("bot.owner")).toString();
			// TODO: more exception types
			} catch(ParseException e) {
				cerr << "parse exception" << endl;
				log << "startup error: " << e.pretty() << endl;
			} catch(StackTrace e) {
				cerr << "stack trace exception" << endl;
				log << "startup error: " << e.toString() << endl;
			} catch(string &e) {
				cerr << "string exception" << endl;
				log << "startup error: " << e << endl;
			}
		}
		return true;
	}
	return false;
}

bool global::deinit() {
	log.close();
	return true;
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
	line = escapeForPMSG(line); // TODO
	if(allSpaces(line))
		return;
	log << " -> " << target << " :" << line << endl;
	unsigned maxLineLength =
		fromString<unsigned>(global::vars.getString("bot.maxLineLength"));
	if(line.length() > maxLineLength) {
		line = line.substr(0, maxLineLength);
		log << "\t(line had to be shortened)" << endl;;
	}
	if(!send)
		log << "\t(didn't really send it, we're being quiet)" << endl;
	else {
		cout << network << " PRIVMSG " << target << " :" << line << endl;
		// TODO: this looks like pbrane is predicting the result in the journal...
		journal::push(journal::Entry(":" +
					global::vars.get("bot.nick").toString() + "!~self@localhost " +
					"PRIVMSG " + target + " :" + line));
	}
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

