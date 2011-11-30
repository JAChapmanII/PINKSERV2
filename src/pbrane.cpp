#include <iostream>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;
using std::getline;

#include <boost/regex.hpp>
using boost::regex;
using boost::smatch;
using boost::regex_match;

#include <fstream>
using std::ofstream;

int main(int argc, char **argv) {
	const string logFileName = "pbrane.log", myNick = "pbrane";
	const string privmsgRegexExp =
		"^:([A-Za-z0-9_]*)!([-@~A-Za-z0-9_\\.]*) PRIVMSG ([#A-Za-z0-9_]*) :(.*)";
	const string joinRegexExp =
		"^:([A-Za-z0-9_]*)!([-@~A-Za-z0-9_\\.]*) JOIN :([#A-Za-z0-9_]*)";

	regex privmsgRegex(privmsgRegexExp, regex::perl);
	regex joinRegex(privmsgRegexExp, regex::perl);

	ofstream log("pbrane.log");
	if(!log.good()) {
		cerr << "Could not open log!" << endl;
		return 1;
	}

	while(!cin.eof()) {
		string line;
		getline(cin, line);

		smatch matches;
		if(regex_match(line, matches, privmsgRegex)) {
			log << "pmsg: " << matches[0] << endl;
			log << "nick: " << matches[1] << endl;
			log << "user: " << matches[2] << endl;
			log << "chan: " << matches[3] << endl;
			log << " msg: " << matches[4] << endl;
			string message(matches[4]);

			const string reloadRegexExp = myNick + "[:,]? +reload";
			regex reloadRegex(reloadRegexExp);
			if(regex_match(message, reloadRegex)) {
				return 77;
			}

		} else if(regex_match(line, matches, joinRegex)) {
			log << "join: " << matches[0] << endl;
			log << "nick: " << matches[1] << endl;
			log << "user: " << matches[2] << endl;
			log << "chan: " << matches[3] << endl;
		} else {
			log << "no match: " << line << endl;
		}
	}

	return 0;
}

