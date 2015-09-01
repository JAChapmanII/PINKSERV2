#include "config.hpp"
using std::string;
using std::vector;

namespace config {
	string nick = "PINKSERV3";
	string startupFile = nick + ".startup";
	string logFileName = nick + ".log";
	string errFileName = nick + ".err";
	string todoFileName = nick + ".todo";
	string journalFileName = nick + ".journal";
	string databaseFileName = nick + ".db";

	namespace regex {
		// TODO: this is a mess
		string toUs = "^(" + config::nick + "[:\\,]?\\s+).*";
		string toUsReplace = "^(" + config::nick + "[:\\,]?\\s+)";
	}

}

