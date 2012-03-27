#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <string>
#include <vector>

namespace global {
	struct ChatLine { // {{{
		std::string nick;
		std::string text;
		ChatLine(std::string inick, std::string itext) :
				nick(inick), text(itext) {
		}
	}; // }}}

	extern std::vector<ChatLine> lastLog;
	extern std::vector<std::string> ignoreList;
}

#endif // GLOBAL_HPP
