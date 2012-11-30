#include "global.hpp"
using global::ExpressionResult;
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
#include "function.hpp"
#include "modules.hpp"
#include "util.hpp"
using util::contains;
using util::split;

ofstream global::log;
ofstream global::err;
mt19937_64 global::rengine;
Dictionary<string, unsigned> global::dictionary;

static ofstream chatFile;
static unsigned int global_seed = 0;

vector<ChatLine> global::lastLog;
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
	return true;
}

void global::secondaryInit() {
	ifstream startup(config::startupFile);
	if(startup.good()) {
		log << "reading startup file: " << config::startupFile << endl;
		ChatLine cl(config::owner, config::nick, "", false, true);
		while(startup.good() && !startup.eof()) {
			getline(startup, cl.text);
			if(startup.eof())
				break;
			if(cl.text.empty())
				continue;
			log << "\t" << cl.text << endl;
			parse(cl);
		}
	}
}

bool global::deinit() {
	log.close();
	err.close();
	return true;
}

bool global::parse(ChatLine line) {
	// primary regex objects
	static regex toUsRegex(config::regex::toUs, regex::perl);
	static regex toUsReplaceRegex(config::regex::toUsReplace, regex::perl);

	if(line.text.empty()) {
		err << "global::parse: line.text.empty?" << endl;
		return false;
	}

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

	if(regex_match(message, toUsRegex)) {
		message = regex_replace(message, toUsReplaceRegex, (string)"");
		chatFile << "\trmsg: " << message << endl;
		line.toUs = true;
	}

	string otarget = line.target;
	if(otarget == config::nick) {
		otarget = line.nick;
		line.toUs = true;
	}

	ExpressionResult res = run(line, message);
	if(!res.result.empty())
		send(otarget, res.result, res.doSend);

	return res.matched;
}

ExpressionResult global::run(ChatLine line, string message, int turtles) {
	// TODO: errors in subexpressions? Include in ExpressionResult
	ExpressionResult ret;
	ret.turtles = ++turtles;
	if(ret.turtles > 6) {
		return ret;
	}

	/* TODO: maybe? Or have !text command?
	size_t tsep = line.text.find(";;");
	if(tsep != string::npos) {
		lastLog.push_back(ChatLine(line.nick, line.target,
					line.text.substr(0, tsep), line.real, line.toUs));
		line.text = trim(line.text.substr(tsep + 2));
	}
	if(line.text.empty())
		return ret;
	*/

	for(size_t subc = message.find("{!"); subc != string::npos;
			subc = message.find("{!")) {
		size_t subcEnd = message.find("}", subc + 2);
		for(size_t subsubStart = message.find("{!", subc + 1);
				subsubStart < subcEnd;
				subsubStart = message.find("{!", subsubStart + 1)) {
			subcEnd = message.find("}", subcEnd + 1);
		}
		if(subcEnd == string::npos) {
			ret.result = "Unmatched brace at: ";
			if(subc > 1)
				ret.result += message.substr(subc - 2, 1);
			if(subc > 0)
				ret.result += message.substr(subc - 1, 1);
			ret.result += message.substr(subc, 5);
			ret.matched = true;
			ret.doSend = true;
			return ret;
		}
		string subexpr = message.substr(subc + 1, subcEnd - subc - 1);
		ExpressionResult subret = run(ChatLine(line.nick, line.target, subexpr,
					line.real, line.toUs), subexpr, turtles);
		// TODO: config constant
		if(subret.turtles > 6) {
			ret.turtles = subret.turtles;
			ret.result = line.nick + ": It's turtles all the way down";
			ret.matched = true;
			ret.doSend = true;
			return ret;
		}
		message = message.substr(0, subc) + subret.result + message.substr(subcEnd + 1);
	}

	boost::smatch matches;
	// loop through setup modules trying to match their regex
	if(contains(ignoreList, line.nick) && !isOwner(line.nick))
		return ret;

	ret.doSend = true;
	for(auto mod : modules::map) {
		regex cmodr(mod.second->regex(), regex::perl);
		// if this module matches
		if(regex_match(message, matches, cmodr, match_extra)) {
			// log that we got a hit
			log << "Module matched: " << mod.first << endl;
			// run the module
			ret.result = mod.second->run(line, matches);
			if(ret.result.empty()) {
				log << "\treturned nothing, moving on" << endl;
			} else {
				// log the output/send the output
				ret.matched = true;
				return ret;
			}
		}
	}

	ret.doSend = time(NULL) > minSpeakTime;
	for(auto mod : modules::map) {
		ret.result = mod.second->secondary(line);
		if(!ret.result.empty()) {
			log << "Module (secondary) matched: " + mod.first << endl;
			ret.matched = true;
			return ret;
		}
	}

	lastLog.push_back(line);

	for(auto module : modules::map) {
		ret.result = module.second->passive(line, ret.matched);
		if(!ret.result.empty()) {
			log << "Module (passive) matched: " + module.first << endl;
			return ret;
		}
	}

	return ret;
}
void global::send(string target, string line, bool send) {
	log << " -> " << target << " :" << line << endl;
	if(line.length() > config::maxLineLength) {
		line = line.substr(0, config::maxLineLength);
		log << "\t(line had to be shortened)" << endl;;
	}
	if(!send)
		log << "\t(didn't really send it, we're being quiet)" << endl;
	else
		cout << "PRIVMSG " << target << " :" << line << endl;
}

bool global::isOwner(std::string nick) {
	return (nick == config::owner);
}
bool global::isAdmin(std::string nick) {
	return contains(config::admins, nick);
}

void global::kick(string from, string nick, string message) {
	log << "kick -> " << nick << " from " << from << " :" << message << endl;
	cout << "KICK " << from << " " << nick << " :" << message << endl;
}

