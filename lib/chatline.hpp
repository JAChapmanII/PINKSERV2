#ifndef CHATLINE_HPP
#define CHATLINE_HPP

#include <string>

struct ChatLine {
	std::string nick;
	std::string target;
	std::string text;
	bool real, toUs;

	ChatLine(std::string inick, std::string itarget, std::string itext,
			bool ireal = true, bool iToUs = false);
	ChatLine();
};

#endif // CHATLINE_HPP
