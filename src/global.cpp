#include "global.hpp"
using std::ofstream;
using std::string;
using std::vector;
using std::fstream;
using std::map;

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
#include "function.hpp"
#include "modules.hpp"
#include "util.hpp"
using util::contains;

static ofstream logFile;
static ofstream errFile;
static ofstream chatFile;

vector<global::ChatLine> global::lastLog;
vector<string> global::ignoreList;
map<string, int> global::siMap;
unsigned global::minSpeakTime = 0;

bool global::init() {
	return true;
}
bool global::deinit() {
	return true;
}

void global::log(string str) {
	if(!logFile.good()) {
		logFile.open(config::logFileName, fstream::app);
		if(!logFile.good()) {
			cerr << str << endl;
			return;
		}
	}
	logFile << str << endl;
}
void global::err(string str) {
	if(!errFile.good()) {
		errFile.open(config::errFileName, fstream::app);
		if(!errFile.good()) {
			cerr << str << endl;
			return;
		}
	}
	errFile << str << endl;
}

bool global::parse(global::ChatLine line) {
	// primary regex objects
	static regex toUsRegex(config::regex::toUs, regex::perl);
	static regex toUsReplaceRegex(config::regex::toUsReplace, regex::perl);

	// log the line, real or fake
	if(!chatFile.good()) {
		chatFile.open(config::chatFileName, fstream::app);
	}
	string lline;
	if(line.real)
		lline += "real: ";
	else
		lline += "fake: ";
	lline += line.nick + "@" + line.target + " :" + line.text;
	if(!chatFile.good())
		cerr << lline << endl;
	else
		chatFile << lline << endl;

	string message = line.text;

	bool toUs = false;
	if(regex_match(message, toUsRegex)) {
		message = regex_replace(message, toUsReplaceRegex, (string)"");
		chatFile << "\trmsg: " << message << endl;
		toUs = true;
	}

	string otarget = line.target;
	if(otarget == config::nick) {
		otarget = line.nick;
		toUs = true;
	}

	// setup function arguments
	FunctionArguments fargs;
	fargs.nick = line.nick;
	fargs.target = line.target;
	fargs.message = line.text;
	fargs.toUs = toUs;
	if(fargs.nick == config::owner)
		fargs.fromOwner = true;

	bool matched = false;
	// loop through setup modules trying to match their regex
	if(!contains(global::ignoreList, fargs.nick)) {
		for(auto mod : modules::map) {
			regex cmodr(mod.second->regex(), regex::perl);
			// if this module matches
			if(regex_match(message, fargs.matches, cmodr, match_extra)) {
				// log that we got a hit
				log("module matched: " + mod.first);
				// run the module
				string res = mod.second->run(fargs);
				if(res.empty()) {
					log("module returned nothing, moving on");
				} else {
					// log the output/send the output
					send(otarget, res);
					matched = true;
					break;
				}
			}
		}
		if(!matched) {
			for(auto mod : modules::map) {
				string res = mod.second->secondary(fargs);
				if(!res.empty()) {
					log("module (secondary) matched " + mod.first);
					//log << matches[1] << "@" << matches[3] << ": " << matches[4] << endl;
					send(otarget, res);
					matched = true;
					break;
				}
			}
		}
	}

	if(!matched)
		lastLog.push_back(line);

	for(auto module : modules::map) {
		string res = module.second->passive(line, matched);
		if(!res.empty()) {
			log("module (passive) matched " + module.first);
			send(otarget, res);
		}
	}

	return matched;
}
void global::send(string target, string line) {
	log(" -> " + target + " :" + line);
	if(line.length() > config::maxLineLength) {
		line = line.substr(0, config::maxLineLength);
		log("    (line had to be shortened)");
	}
	if(time(NULL) < minSpeakTime)
		log("    (didn't really send it, we're being quiet)");
	else
		cout << "PRIVMSG " << target << " :" << line << endl;
}

