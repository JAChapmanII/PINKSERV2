#include "config.hpp"
using std::string;

namespace config {
	string nick = "PINKSERV2";
	string owner = "jac";
	string reload = nick + ": reload";
	string logFileName = nick + ".log";
	string errFileName = nick + ".err";
	string chatFileName = nick + ".chat";
	string todoFileName = nick + ".todo";
	string brainFileName = nick + ".brain";
	unsigned int maxLineLength = 512;
	double markovResponseChance = 0.08;

	namespace regex {
		string user = "([A-Za-z0-9_]*)";
		string hmask = "([-/@~A-Za-z0-9_\\.]*)";
		string target = "([#A-Za-z0-9_]*)";

		string privmsg = "^:" + user + "!" + hmask + " PRIVMSG " + target + " :(.*)";
		string join = "^:" + user + "!" + hmask + " JOIN :?" + target;
		string toUs = "^(" + config::nick + "[:\\,]?\\s+).*";
		string toUsReplace = "^(" + config::nick + "[:\\,]?\\s+)";
	}

}

