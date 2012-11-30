#include "regex.hpp"
using std::string;
using std::vector;
using boost::regex;
using boost::smatch;

#include <sstream>
using std::stringstream;

#include <exception>
using std::exception;

#include <map>
using std::map;

#include <boost/regex.hpp>
using boost::regex_match;
using boost::match_extra;

#include "util.hpp"
using util::contains;

#include "global.hpp"
#include "modules.hpp"
#include "brain.hpp"
using std::ostream;
using std::istream;
#include "config.hpp"

static map<string, PredefinedRegexFunction *> prfs;

// strip \r and \n from the output
string cleanWith(string dirty);
string cleanWith(string dirty) { // {{{
	boost::regex clean("(\\r|\\n|\\\\r|\\\\n)", regex::perl);
	return regex_replace(dirty, clean, "",
			boost::match_default | boost::format_all);
} // }}}


string RegexFunction::run(ChatLine line, smatch matches) { // {{{
	string replace = matches[2], with = cleanWith(matches[3]), nick = matches[5];
	try {
		boost::regex rregex(replace, regex::perl);
		for(auto i = global::lastLog.rbegin(); i != global::lastLog.rend(); ++i) {
			string str = regex_replace(i->text, rregex, with,
					boost::match_default | boost::format_all);
			if(!nick.empty() && (nick != i->nick))
				continue;
			if(str != i->text) {
				string result = "";
				// TODO this is kind of a hack...
				if(!i->nick.empty())
					result += "<" + i->nick + "> ";
				result += str;
				if(str.length() > config::maxLineLength)
					str = str.substr(0, config::maxLineLength);
				global::lastLog.push_back(ChatLine(
							i->nick, line.target, str, false));
				return result;
			}
		}
		return line.nick + ": error: not matched";
	} catch(exception &e) {
		return line.nick + ": error: " + e.what();
	}
} // }}}
string RegexFunction::name() const { // {{{
	return "regex";
} // }}}
string RegexFunction::help() const { // {{{
	return (string)"Give it two regex, it finds the last message that" +
		" matches the first and substitutes it with the second. You can" +
		" append a nick to the very end to restrict the match further";
} // }}}
string RegexFunction::regex() const { // {{{
	return "^(!)?s/([^/]+)/([^/]*)(/(.+)?)?$";
} // }}}


PredefinedRegexFunction::PredefinedRegexFunction(string iname) : // {{{
		Function(true),
		m_name(iname), m_first(), m_second(), m_replaces() {
} // }}}
PredefinedRegexFunction::PredefinedRegexFunction(string iname, // {{{
		vector<string> first, vector<string> second) : Function(true),
		m_name(iname), m_first(), m_second(), m_replaces() {
	for(unsigned i = 0; i < first.size(); ++i)
		this->push(first[i], second[i]);
} // }}}
PredefinedRegexFunction::PredefinedRegexFunction(string iname, // {{{
		string first, string second) : Function(true),
		m_name(iname), m_first(), m_second(), m_replaces() {
	this->push(first, second);
} // }}}
string PredefinedRegexFunction::push(string first, string second) { // {{{
	try {
		boost::regex reg(first, regex::perl);
		this->m_first.push_back(first);
		this->m_replaces.push_back(reg);
		this->m_second.push_back(second);
	} catch(exception &e) {
		return e.what();
	}
	return "";
} // }}}
string PredefinedRegexFunction::run(ChatLine line, smatch matches) { // {{{
	if(this->m_replaces.empty())
		return line.nick + ": no regex in replaces";

	string m2 = matches[1];
	if(m2.empty())
		m2 = ".*";

	try {
		boost::regex matchreg(m2, regex::perl);
		for(auto i = global::lastLog.rbegin(); i != global::lastLog.rend(); ++i) {
			if(regex_match(i->text, matchreg)) {
				string str = i->text, nick = i->nick;
				for(unsigned j = 0; j < this->m_replaces.size(); ++j) {
					str = regex_replace(str, this->m_replaces[j],
							this->m_second[j], boost::match_default | boost::format_all);
				}
				if(str.length() > config::maxLineLength)
					str = str.substr(0, config::maxLineLength);
				global::lastLog.push_back(ChatLine(nick, line.target, str, false));
				return (string)"<" + nick + "> " + str;
			}
		}
		return line.nick + ": error: not matched";
	} catch(exception &e) {
		return line.nick + ": error: " + e.what();
	}
	return line.nick + ": what the fuck I skipped the try/catch block!?";
} // }}}
string PredefinedRegexFunction::name() const { // {{{
	return this->m_name;
} // }}}
string PredefinedRegexFunction::help() const { // {{{
	return this->m_name + " replaces \"" + this->m_first[0] + "\" with \"" +
		this->m_second[0] + "\" and maybe others";
} // }}}
string PredefinedRegexFunction::regex() const { // {{{
	return "^!" + this->m_name + "(?:\\s+(.*))?";
} // }}}
ostream &PredefinedRegexFunction::output(ostream &out) { // {{{
	unsigned char size = this->m_first.size();
	out << size;
	for(unsigned i = 0; i < size; ++i) {
		brain::write(out, this->m_first[i]);
		brain::write(out, this->m_second[i]);
	}
	return out;
} // }}}
istream &PredefinedRegexFunction::input(istream &in) { // {{{
	int size = in.get();
	if(size < 0)
		return in;
	for(int i = 0; i < size; ++i) {
		string first, second;
		brain::read(in, first);
		brain::read(in, second);
		this->push(first, second);
	}
	return in;
} // }}}


PushFunction::PushFunction() : Function(true) { // {{{
} // }}}
string PushFunction::run(ChatLine line, smatch matches) { // {{{
	string fname = matches[1], first = matches[2], second = matches[3];

	if(contains(prfs, fname)) {
		// user wants to erase a function
		if(first.empty() && second.empty()) {
			delete prfs[fname];
			prfs.erase(prfs.find(fname));
			return line.nick + ": " + fname + " erased";
		}
		// if they left off first (replace nothing with something)
		if(first.empty())
			return line.nick + ": error: can't replace nothing";
		PredefinedRegexFunction *func = prfs[fname];
		string ret = func->push(first, second);
		if(ret.empty())
			return line.nick + ": added new regex to " + fname;
		return line.nick + ": error: " + ret;
	} else {
		// if it's already in the normal map, error
		if(contains(modules::map, fname))
			return line.nick + ": error: function by that name already exists";

		// try to create a new PRF object
		PredefinedRegexFunction *func = new PredefinedRegexFunction(fname);
		if(func == NULL)
			return line.nick + ": error: couldn't create new object";

		// try to add the initial regex to it
		string ret = func->push(first, second);
		if(!ret.empty()) {
			delete func;
			return line.nick + ": error: " + ret;
		}

		// add it to the prfs map
		prfs[fname] = func;
		return line.nick + ": " + fname + " added to list";
	}

	return line.nick + ": error: shouldn't have gotten here";
} // }}}
string PushFunction::name() const { // {{{
	return "push";
} // }}}
string PushFunction::help() const { // {{{
	return "Dynamically adds a PredefinedRegex function to the module map";
} // }}}
string PushFunction::regex() const { // {{{
	return "^!push/([^/]+)/([^/]*)/([^/]*)/?$";
} // }}}
ostream &PushFunction::output(ostream &out) { // {{{
	unsigned size = prfs.size();
	brain::write(out, size);
	for(auto prf : prfs) {
		string fname = prf.first;
		PredefinedRegexFunction *f = prf.second;

		brain::write(out, fname);
		out << *f;
	}
	return out;
} // }}}
istream &PushFunction::input(istream &in) { // {{{
	unsigned size = 0;
	brain::read(in, size);
	for(unsigned i = 0; i < size; ++i) {
		string fname;
		brain::read(in, fname);

		PredefinedRegexFunction *f = new PredefinedRegexFunction(fname);
		in >> *f;
		prfs[fname] = f;
	}
	return in;
} // }}}


string InvokeFunction::secondary(ChatLine line) {
	smatch matches;
	for(auto i : prfs) {
		boost::regex cmodr(i.second->regex(), regex::perl);
		// if this prf matches
		if(regex_match(line.text, matches, cmodr, match_extra)) {
			// run the module
			string res = i.second->run(line, matches);
			if(!res.empty()) {
				return res;
			}
		}
	}
	return "";
}
string InvokeFunction::name() const {
	return "invoke";
}
string InvokeFunction::help() const {
	return "Magically called for !push'd functions :)";
}
string InvokeFunction::regex() const {
	return "";
}


string ListRegexesFunction::run(ChatLine line, smatch matches) { // {{{
	string function = matches[2];
	if(!function.empty()) {
		if(!contains(prfs, function))
			return line.nick + ": that function doesn't exist";
		else
			return line.nick + ": " + prfs[function]->help();
	}

	string list;
	for(auto i : prfs)
		list += i.second->name() + ", ";
	return line.nick + ": " + list.substr(0, list.length() - 2);
} // }}}
string ListRegexesFunction::name() const { // {{{
	return "rlist";
} // }}}
string ListRegexesFunction::help() const { // {{{
	return "List the predefined regex functions";
} // }}}
string ListRegexesFunction::regex() const { // {{{
	return "^!rlist(\\s+(\\S+)?)?";
} // }}}

