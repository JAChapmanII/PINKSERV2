#include "regex.hpp"
using std::string;
using std::vector;
using boost::regex;

#include <sstream>
using std::stringstream;

#include <exception>
using std::exception;

#include "util.hpp"
using util::split;
using util::contains;

#include "global.hpp"
#include "modules.hpp"

/*
	// load old pushes if they exists {{{
	in.open(pushFileName);
	if(in.good()) {
		log << "reading old push entries" << endl;

		FunctionArguments fargs;
		fargs.nick = ownerNick;
		fargs.user = (string)"~" + ownerNick;
		fargs.target = myNick;
		fargs.toUs = true;
		fargs.siMap = NULL;
		fargs.fromOwner = true;
		regex mreg(modules::map["push"]->regex(), regex::perl);

		unsigned lcount = 0;
		while(!in.eof()) {
			string line;
			getline(in, line);
			if(in.eof() || !in.good())
				break;
			lcount++;

			// if this module matches
			if(regex_match(line, fargs.matches, mreg, match_extra)) {
				fargs.message = line;
				string res = modules::map["push"]->run(fargs);
				if(!res.empty()) {
					// log the output/send the output
					log << " ?> " << fargs.target << " :" << res << endl;
				}
			}
		}
		log << endl << pushFileName << ": read " << lcount << " lines" << endl;

		stringstream ss;
		ss << "Read " << lcount << " lines ";
		unsigned j = 0, last = modules::map.size() - 1;
		for(auto i : modules::map) {
			ss << i.second->name();
			if(j != last)
				ss << ", ";
			++j;
		}
		string res = ss.str();
		log << " -> " << ownerNick << " : " << res << endl;
		cout << "PRIVMSG " << ownerNick << " :" << res << endl;
	}
	in.close();
	// }}}
*/

string ReplaceFunction::run(FunctionArguments fargs) { // {{{
	string m2 = fargs.matches[1], m4 = fargs.matches[2];

	for(auto i = global::lastLog.rbegin(); i != global::lastLog.rend(); ++i) {
		string str = i->text;
		vector<string> words = split(str, " \t");
		if(contains(words, m2)) {
			stringstream ss;
			for(auto j = words.begin(); j != words.end(); ++j) {
				if(*j == m2)
					ss << m4;
				else
					ss << *j;
				if(j != words.end() - 1)
					ss << " ";
			}
			global::lastLog.push_back(global::ChatLine(i->nick, ss.str()));
			return (string)"<" + i->nick + "> " + ss.str();
		}
	}

	return fargs.nick + ": error: not matched";
} // }}}
string ReplaceFunction::name() const { // {{{
	return "replace";
} // }}}
string ReplaceFunction::help() const { // {{{
	return "Replace one word with a string";
} // }}}
string ReplaceFunction::regex() const { // {{{
	return "^!s\\s+([^\\s]+)\\s+(.+)\\s*$";
} // }}}


string RegexFunction::run(FunctionArguments fargs) { // {{{
	string m2 = fargs.matches[1], m4 = fargs.matches[2];

	try {
		boost::regex rgx(m2, regex::perl);
		for(auto i = global::lastLog.rbegin(); i != global::lastLog.rend(); ++i) {
			string str = regex_replace(i->text, rgx, m4,
					boost::match_default | boost::format_all);
			if(str != i->text) {
				global::lastLog.push_back(global::ChatLine(i->nick, str));
				return (string)"<" + i->nick + "> " + str;
			}
		}
		return fargs.nick + ": error: not matched";
	} catch(exception &e) {
		return fargs.nick + ": error: " + e.what();
	}
} // }}}
string RegexFunction::name() const { // {{{
	return "regex";
} // }}}
string RegexFunction::help() const { // {{{
	return (string)"Give it two regex, it finds the last message that" +
		" matches the first and substitutes it with the second";
} // }}}
string RegexFunction::regex() const { // {{{
	return "^!s/([^/]+)/([^/]*)/?$";
} // }}}


PredefinedRegexFunction::PredefinedRegexFunction(string iname) : // {{{
		m_name(iname), m_first(), m_second(), m_replaces() {
} // }}}
PredefinedRegexFunction::PredefinedRegexFunction(string iname, // {{{
		vector<string> first, vector<string> second) :
		m_name(iname), m_first(), m_second(), m_replaces() {
	for(unsigned i = 0; i < first.size(); ++i)
		this->push(first[i], second[i]);
} // }}}
PredefinedRegexFunction::PredefinedRegexFunction(string iname, // {{{
		string first, string second) :
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
string PredefinedRegexFunction::run(FunctionArguments fargs) { // {{{
	if(this->m_replaces.empty())
		return fargs.nick + ": no regex in replaces";

	string m2 = fargs.matches[1];
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
				global::lastLog.push_back(global::ChatLine(nick, str));
				return (string)"<" + nick + "> " + str;
			}
		}
		return fargs.nick + ": error: not matched";
	} catch(exception &e) {
		return fargs.nick + ": error: " + e.what();
	}
	return fargs.nick + ": what the fuck I skipped the try/catch block!?";
}
string PredefinedRegexFunction::name() const {
	return this->m_name;
} // }}}
string PredefinedRegexFunction::help() const { // {{{
	return "Makes something sound " + this->m_name + "-ish";
} // }}}
string PredefinedRegexFunction::regex() const { // {{{
	return "^!" + this->m_name + "(?:\\s+(.*))?";
} // }}}


PushFunction::PushFunction() : m_functions() { // {{{
} // }}}
string PushFunction::run(FunctionArguments fargs) { // {{{
	string fname = fargs.matches[1], first = fargs.matches[2],
			 second = fargs.matches[3];

	if(contains(this->m_functions, fname)) {
		if(first.empty() && second.empty()) {
			auto it = find(this->m_functions.begin(),
					this->m_functions.end(), fname);
			if(it == this->m_functions.end())
				return fargs.nick + ": " + fname + " does not exist";
			this->m_functions.erase(it);
			auto it2 = modules::map.find(fname);
			if(it2 == modules::map.end())
				return fargs.nick + ": " + fname + " not found in modules::map";
			modules::map.erase(it2);
			return fargs.nick + ": " + fname + " erased";
		}
		if(first.empty())
			return fargs.nick + ": error: first is empty";
		PredefinedRegexFunction *func = (PredefinedRegexFunction *)modules::map[fname];
		string ret = func->push(first, second);
		if(ret.empty())
			return fargs.nick + ": added new regex to " + fname;
		return fargs.nick + ": error: " + ret;
	} else {
		if(contains(modules::map, fname))
			return fargs.nick + ": error: function by that name already exists";
		PredefinedRegexFunction *func = new PredefinedRegexFunction(fname);
		if(func == NULL)
			return fargs.nick + ": error: couldn't create new object";
		string ret = func->push(first, second);
		if(!ret.empty()) {
			delete func;
			return fargs.nick + ": error: " + ret;
		}
		this->m_functions.push_back(fname);
		modules::map[fname] = func;
		return fargs.nick + ": " + fname + " added to module map";
	}

	return fargs.nick + ": error: shouldn't have gotten here";
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


string PushXMLFunction::regex() const { // {{{
	return "^!push~([^~]+)~([^~]*)~([^~]*)~?$";
} // }}}

