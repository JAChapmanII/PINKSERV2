#include "script.hpp"
using std::string;
using boost::regex;
using boost::regex_match;
using boost::match_extra;
using boost::smatch;
using std::ostream;
using std::istream;

#include <vector>
using std::vector;

#include <exception>
using std::exception;

#include "global.hpp"
#include "util.hpp"
using util::join;
using util::contains;
using util::split;
#include "brain.hpp"

static string lastTrigger;

OnRegexFunction::OnRegexFunction() : m_triggers(), m_scopes(), m_regex(), m_lines() {
}

string OnRegexFunction::run(FunctionArguments fargs) {
	string rstring = fargs.matches[1], scope = fargs.matches[2],
			line = fargs.matches[3];
	try {
		// TODO: unmagic this ( for example, what about (^|\s) ?)
		if(rstring[0] != '^')
			rstring = ".*" + rstring;
		if(rstring[rstring.length() - 1] != '$')
			rstring += ".*";

		boost::regex rgx(rstring, regex::perl);

		global::ChatLine cl(fargs.nick, fargs.target, line, false);

		this->m_triggers.push_back(rstring);
		this->m_scopes.push_back(scope);
		this->m_regex.push_back(rgx);
		this->m_lines.push_back(cl);
		return fargs.nick + ": will do!";
	} catch(exception &e) {
		return fargs.nick + ": error: " + e.what();
	}
	return fargs.nick + ": oh god what happened?";
}

string OnRegexFunction::secondary(FunctionArguments fargs) {
	smatch matches;
	for(unsigned i = 0; i < this->m_regex.size(); ++i) {
		if(!this->m_scopes[i].empty()) {
			vector<string> nicks = split(this->m_scopes[i], ",");
			if(!contains(nicks, fargs.nick))
				continue;
		}
		if(regex_match(fargs.message, matches, this->m_regex[i], match_extra)) {
			global::ChatLine cl = this->m_lines[i];
			cl.target = fargs.target;
			if(global::parse(cl))
				lastTrigger = this->m_triggers[i];
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
	return "^!on\\s+/([^/]+)/(\\S*)\\s+(.+)$";
}

ostream &OnRegexFunction::output(ostream &out) {
	unsigned size = this->m_triggers.size();
	brain::write(out, size);
	for(unsigned i = 0; i < size; ++i) {
		brain::write(out, this->m_triggers[i]);
		brain::write(out, this->m_scopes[i]);
		brain::write(out, this->m_lines[i]);
	}
	return out;
}
istream &OnRegexFunction::input(istream &in) {
	unsigned size = 0;
	brain::read(in, size);
	for(unsigned i = 0; i < size; ++i) {
		string trigger, scope;
		global::ChatLine cl;
		brain::read(in, trigger);
		brain::read(in, scope);
		brain::read(in, cl);

		try {
			boost::regex rgx(trigger, regex::perl);
			this->m_triggers.push_back(trigger);
			this->m_scopes.push_back(scope);
			this->m_regex.push_back(rgx);
			this->m_lines.push_back(cl);
		} catch(exception &e) {
			;//
		}
	}
	return in;
}


string ExplainFunction::run(FunctionArguments fargs) {
	return fargs.nick + ": that was from " + lastTrigger;
}
string ExplainFunction::name() const {
	return "explain";
}
string ExplainFunction::help() const {
	return "Explains the last thing we said from !on";
}
string ExplainFunction::regex() const {
	return "^!explain(\\s+.*)?";
}

