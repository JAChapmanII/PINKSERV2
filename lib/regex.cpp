#include "regex.hpp"
using std::string;
using boost::regex;
using boost::regex_replace;

#include <exception>
using std::exception;

Regex::Regex(string str) { parse(str); }

RegexType Regex::type() const { return _type; }
string Regex::match() const { return _match; }
string Regex::flags() const { return _flags; }

bool Regex::matches(string text) const {
	boost::match_results<string::const_iterator> what;
	// TODO: last arg is match_flag_type ? defaults to match_default
	return regex_search(text, what, _regex);
}

void Regex::parse(string str) {
	if(str.length() < 3) // must be at least /./
		throw (string)"Regex::parse: error: str is too short";

	if(str[0] == str[1]) // this implies //
		throw (string)"Regex::parse: error: empty search is invalid";
	string orig = str;

	_delimiter = str.front();
	str = str.substr(1); // strip off first delimiter

	size_t rend = 0;
	// find the end of the first portion (search)
	for(rend = 1; rend < str.length(); ++rend) {
		if(str[rend] == '\\')
			rend++;
		else if(str[rend] == _delimiter)
			break;
	}
	// it could be past the end if the last char was a backslash
	if(rend >= str.length())
		throw (string)"Regex::parse: unterminated regex: " + orig;

	_replace = str.substr(rend + 1);
	_match = str.substr(0, rend);
	_type = RegexType::Match;

	// if we have a replacement, we may have flags
	if(!_replace.empty()) {
		_type = RegexType::Replace;
		if(_replace.front() == _delimiter) {
			_flags = _replace.substr(1);
			_replace = "";
		} else {
			// search for the end of the replace part
			for(rend = 0; rend < _replace.length(); ++rend) {
				if(_replace[rend] == '\\')
					rend++;
				else if(_replace[rend] == _delimiter)
					break;
			}
			// if we found another delimiter, then we have flags
			if(rend != _replace.length()) {
				_flags = _replace.substr(rend + 1);
				_replace = _replace.substr(0, rend);
			}
		}
	}

	// TODO: parse flags
	try {
		_regex = regex(_match, regex::perl);
	} catch(exception &e) {
		// TODO: better handling?
		throw (string)"error: " + e.what();
	}
}

bool Regex::execute(string text, string &result) const {
	// TODO: group variables, r0, r1, etc
	result = regex_replace(text, _regex, _replace,
			boost::match_default | boost::format_all); // TODO: flags
	return matches(text);
}

