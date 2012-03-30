#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <fstream>
#include <string>
#include <vector>
#include <map>

namespace global {
	extern std::ofstream log;
	extern std::ofstream err;

	struct ChatLine { // {{{
		std::string nick;
		std::string target;
		std::string text;
		bool real;
		ChatLine(std::string inick, std::string itarget, std::string itext,
				bool ireal = true) :
				nick(inick), target(itarget), text(itext), real(ireal) {
		}
		ChatLine() : nick(), target(), text(), real(false) {
		}
	}; // }}}

	bool init();
	bool deinit();

	extern std::vector<ChatLine> lastLog;
	extern std::vector<std::string> ignoreList;
	extern std::map<std::string, int> siMap;
	extern unsigned minSpeakTime;

	bool parse(ChatLine line);
	void send(std::string target, std::string line);
}

#endif // GLOBAL_HPP
