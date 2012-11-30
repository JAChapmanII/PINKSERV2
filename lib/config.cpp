#include "config.hpp"
using std::string;
using std::vector;

namespace config {
	string nick = "Pinky2";
	string owner = "jac";
	string startupFile = nick + ".startup";
	vector<string> admins = { "jac" };
	string reload = nick + ": reload";
	string die = nick + ": die";
	string logFileName = nick + ".log";
	string errFileName = nick + ".err";
	string chatFileName = nick + ".chat";
	string todoFileName = nick + ".todo";
	string brainFileName = nick + ".brain";
	unsigned int maxLineLength = 512;
	double markovResponseChance = 0.08;
	double correctionResponseChance = 0.08;

	namespace regex {
		string user = "([A-Za-z0-9_]*)";
		string hmask = "([-/@~A-Za-z0-9_\\.]*)";
		// TODO: this is wrong, & other prefixes?
		string target = "([#A-Za-z0-9_]*)";
		string value = "(.*)";
		string message = "(:"+ value + ")";

		string privmsg = "^:" + user + "!" + hmask + " PRIVMSG " + target + " :(.*)";
		// TODO: this is a mess
		string join = "^:" + user + "!" + hmask + " JOIN " + value + " " + value + "?";
		string quit = "^:" + user + "!" + hmask + " QUIT " + message + "?";
		string toUs = "^(" + config::nick + "[:\\,]?\\s+).*";
		string toUsReplace = "^(" + config::nick + "[:\\,]?\\s+)";
	}

}

