#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <random>

namespace global {
	extern std::ofstream log;
	extern std::ofstream err;
	extern std::mt19937_64 rengine;

	struct ChatLine { // {{{
		std::string nick;
		std::string target;
		std::string text;
		bool real, toUs;
		ChatLine(std::string inick, std::string itarget, std::string itext,
				bool ireal = true, bool iToUs = false) :
				nick(inick), target(itarget), text(itext), real(ireal), toUs(iToUs) {
		}
		ChatLine() : nick(), target(), text(), real(false), toUs(false) {
		}
	}; // }}}

	bool init(unsigned int seed);
	bool deinit();

	extern std::vector<ChatLine> lastLog;
	extern std::vector<std::string> ignoreList;
	extern unsigned minSpeakTime;

	bool parse(ChatLine line);
	void send(std::string target, std::string line, bool send = true);

	bool isOwner(std::string nick);
	bool isAdmin(std::string nick);


	void kick(std::string from, std::string nick, std::string message = "");
}

#endif // GLOBAL_HPP
