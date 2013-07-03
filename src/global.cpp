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

ofstream global::log;
ofstream global::err;
mt19937_64 global::rengine;
Dictionary<string, unsigned> global::dictionary;
EventSystem global::eventSystem;

map<string, Variable> global::vars;
map<string, map<string, Variable>> global::lvars;

static ofstream chatFile;
static unsigned int global_seed = 0;

vector<string> global::ignoreList;
unsigned global::minSpeakTime = 0;

bool global::init(unsigned int seed) {
	// handle seeding the random number engine
	global_seed = seed;
	rengine.seed(seed);

	log.open(config::logFileName, fstream::app);
	if(!log.good()) {
		cerr << "global::init: could not open log file!" << endl;
		return false;
	}
	err.open(config::errFileName, fstream::app);
	if(!err.good()) {
		cerr << "global::init: could not open error file!" << endl;
		return false;
	}

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
		log << "\tTODO re-implement" << endl;
		while(startup.good() && !startup.eof()) {
			// TODO: make fake logitem?
			string line;
			getline(startup, line);
			if(startup.eof())
				break;
			if(line.empty())
				continue;
			log << "\t" << line << endl;
			//parse(cl);
		}
		return true;
	}
	return false;
}

bool global::deinit() {
	log.close();
	err.close();
	return true;
}

void global::send(string target, string line, bool send) {
	log << " -> " << target << " :" << line << endl;
	unsigned maxLineLength =
		fromString<unsigned>(global::vars["bot.maxLineLength"].toString());
	if(line.length() > maxLineLength) {
		line = line.substr(0, maxLineLength);
		log << "\t(line had to be shortened)" << endl;;
	}
	if(!send)
		log << "\t(didn't really send it, we're being quiet)" << endl;
	else
		cout << "PRIVMSG " << target << " :" << line << endl;
}

bool global::isOwner(std::string nick) {
	return (nick == global::vars["bot.owner"].toString());
}
bool global::isAdmin(std::string nick) {
	return contains(global::vars["bot.admins"].toString(), " " + nick + " ");
}

uint64_t global::now() {
	// TODO: fix
	return time(NULL);
}

