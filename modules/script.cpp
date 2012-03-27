#include "script.hpp"
using std::string;
using boost::regex;
using boost::regex_match;
using boost::match_extra;
using boost::smatch;

#include <exception>
using std::exception;

#include "global.hpp"
#include "util.hpp"
using util::join;
using util::contains;

OnRegexFunction::OnRegexFunction() : m_regex(), m_lines() {
}

string OnRegexFunction::run(FunctionArguments fargs) {
	string rstring = fargs.matches[1], line = fargs.matches[2];
	try {
		boost::regex rgx(rstring, regex::perl);

		global::ChatLine cl(fargs.nick, fargs.target, line, false);

		this->m_regex.push_back(rgx);
		this->m_lines.push_back(cl);
		return fargs.nick + ": will do!";
	} catch(exception &e) {
		return fargs.nick + ": error: " + e.what();
	}
	return fargs.nick + ": oh god what happened?";
}

#include <iostream>
using std::cerr;
using std::endl;

string OnRegexFunction::secondary(FunctionArguments fargs) {
	smatch matches;
	for(unsigned i = 0; i < this->m_regex.size(); ++i) {
		if(regex_match(fargs.message, matches, this->m_regex[i], match_extra)) {
			global::ChatLine cl = this->m_lines[i];
			cl.target = fargs.target;
			global::parse(cl);
		}
	}
	return "";
}

string OnRegexFunction::name() const {
	return "on";
}
string OnRegexFunction::help() const {
	return "When a regex matches, simulate a typed line";
}
string OnRegexFunction::regex() const {
	return "^!on\\s+/([^/]+)/\\s+(.+)$";
}

