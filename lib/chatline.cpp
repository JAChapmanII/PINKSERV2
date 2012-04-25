#include "chatline.hpp"
using std::string;

ChatLine::ChatLine(string inick, string itarget, string itext,
		bool ireal, bool iToUs) :
		nick(inick), target(itarget), text(itext), real(ireal), toUs(iToUs) {
}

ChatLine::ChatLine() : nick(), target(), text(), real(false), toUs(false) {
}

