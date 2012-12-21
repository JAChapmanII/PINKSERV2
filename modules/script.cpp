#include "script.hpp"
using std::string;
using std::ostream;
using std::istream;
using boost::regex;
using boost::regex_match;
using boost::match_extra;
using boost::smatch;

#include <vector>
using std::vector;

#include <exception>
using std::exception;

#include <random>
using std::uniform_int_distribution;

#include <ctime>

#include "global.hpp"
#include "util.hpp"
using util::join;
using util::contains;
using util::split;
#include "brain.hpp"

static string lastTrigger;

OnRegexFunction::OnRegexFunction() :
		Function("on", "When a regex matches, simulate a typed line",
				"^!on\\s+/([^/]+)/(\\S*)\\s+(.+)$"),
		m_triggers(), m_scopes(), m_regex(), m_lines() {
}

string OnRegexFunction::run(ChatLine line, smatch matches) {
	string rstring = matches[1], scope = matches[2], text = matches[3];
	try {
		// TODO: unmagic this ( for example, what about (^|\s) ?)
		if(rstring[0] != '^')
			rstring = ".*" + rstring;
		if(rstring[rstring.length() - 1] != '$')
			rstring += ".*";

		boost::regex rgx(rstring, regex::perl);

		ChatLine cl(line.nick, line.target, text, false);

		this->m_triggers.push_back(rstring);
		this->m_scopes.push_back(scope);
		this->m_regex.push_back(rgx);
		this->m_lines.push_back(cl);
		return line.nick + ": will do!";
	} catch(exception &e) {
		return line.nick + ": error: " + e.what();
	}
	return line.nick + ": oh god what happened?";
}

string OnRegexFunction::secondary(ChatLine line) {
	if(time(NULL) <= global::minSpeakTime)
		return "";
	smatch matches;
	for(unsigned i = 0; i < this->m_regex.size(); ++i) {
		if(!this->m_scopes[i].empty()) {
			vector<string> nicks = split(this->m_scopes[i], ",");
			if(!contains(nicks, line.nick))
				continue;
		}
		if(regex_match(line.text, matches, this->m_regex[i], match_extra)) {
			ChatLine cl = this->m_lines[i];
			cl.target = line.target;

			try {
				boost::regex matchreg("\\$user", regex::perl);
				global::err << "trying to do replace" << std::endl;
				global::err << "cl.text: " << cl.text << std::endl;
				cl.text = regex_replace(cl.text, matchreg, line.nick,
						boost::match_default | boost::format_all);
				global::err << "cl.text: " << cl.text << std::endl;
			} catch(exception &e) {
				global::err << "had excpetion in on regex:" << e.what() << std::endl;
			}

			if(global::parse(cl))
				lastTrigger = this->m_triggers[i];
		}
	}
	return "";
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
		ChatLine cl;
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


ExplainFunction::ExplainFunction() : Function( // {{{
		"explain", "Explains the last thing we said from !on",
		"^!explain(\\s+.*)?") {
} // }}}
string ExplainFunction::run(ChatLine line, smatch matches) { // {{{
	return line.nick + ": that was from " + lastTrigger;
} // }}}


static int gun_next = 6;
static void spin_gun() {
	uniform_int_distribution<> uid(0, 5);
	gun_next = uid(global::rengine);
}

RouletteFunction::RouletteFunction() : Function( // {{{
		"roulette", "Russian roulette!", "^!roulette(\\s.*)?") {
} // }}}
string RouletteFunction::run(ChatLine line, smatch matches) { // {{{
	if(gun_next >= 6) {
		spin_gun();
	}
	++gun_next;
	if(gun_next == 6) {
		global::kick(line.target, line.nick, "BANG!");
		return line.nick + " shot himself, hehehe :D";
	}
	return line.nick + ": ... click!";
} // }}}

SpinFunction::SpinFunction() : Function( // {{{
		"spin", "Spin the barrel of the russian roulette game",
		"^!spin(\\s.*)?") {
} // }}}
string SpinFunction::run(ChatLine line, smatch matches) { // {{{
	spin_gun();
	return line.nick +
		": round and round it goes, where it stopped nobody knows!";
} // }}}

TextFunction::TextFunction() : Function( // {{{
		"text", "Make me see text", "^!text\\s(.*)") {
} // }}}
string TextFunction::run(ChatLine line, smatch matches) { // {{{
	global::lastLog.push_back(
			ChatLine("", line.target, matches[1], false, false));
	// TODO: status codes so this doesn't need to be hacked >_>
	return " ";
} // }}}

