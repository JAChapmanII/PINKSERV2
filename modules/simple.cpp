#include "simple.hpp"
using std::string;

#include <sstream>
using std::stringstream;

#include <queue>
using std::queue;

#include <vector>
using std::vector;

#include <boost/regex.hpp>
using boost::regex;
using boost::smatch;

string WaveFunction::run(FunctionArguments fargs) { // {{{
	if(fargs.message[0] == 'o')
		return "\\o";
	else
		return "o/";
} // }}}
string WaveFunction::name() const { // {{{
	return "wave";
} // }}}
string WaveFunction::help() const { // {{{
	return "Takes no arguments; waves.";
} // }}}
string WaveFunction::regex() const { // {{{
	return "^(o/|\\\\o)( .*)?";
} // }}}


string LoveFunction::run(FunctionArguments fargs) { // {{{
	if(fargs.message[1] == '/')
		return ":(";
	return "<3";
} // }}}
string LoveFunction::name() const { // {{{
	return "</?3";
} // }}}
string LoveFunction::help() const { // {{{
	return "Takes no arguments; outputs love.";
} // }}}
string LoveFunction::regex() const { // {{{
	return "^</?3( .*)?";
} // }}}


string FishFunction::run(FunctionArguments fargs) { // {{{
	int fcount = 1;
	if((fargs.message.length() >= 5) && (fargs.message[5] == 'e'))
		fcount = rand() % 6 + 2;

	stringstream ss;
	for(int i = 0; i < fcount; ++i) {
		if(rand() % 2)
			ss << "<><";
		else
			ss << "><>";
		if(i != fcount - 1)
			ss << string(rand() % 3 + 1, ' ');
	}
	return ss.str();
} // }}}
string FishFunction::name() const { // {{{
	return "fish(es)?";
} // }}}
string FishFunction::help() const { // {{{
	return "Takes no arguments; outputs fish(es).";
} // }}}
string FishFunction::regex() const { // {{{
	return "^!fish(es)?( .*)?";
} // }}}


string TrainFunction::run(FunctionArguments fargs) { // {{{
	int ccount = rand() % 8, dir = rand() % 2;
	stringstream ss;
	if(dir)
		ss << "/.==.]";
	else
		ss << "{. .}";
	for(int i = 0; i < ccount; ++i)
		ss << "[. .]";
	if(dir)
		ss << "{. .}";
	else
		ss << "[.==.\\";

	return ss.str();
} // }}}
string TrainFunction::name() const { // {{{
	return "sl";
} // }}}
string TrainFunction::help() const { // {{{
	return "Takes no arguments; returns a train.";
} // }}}
string TrainFunction::regex() const { // {{{
	return "^!sl( .*)?";
} // }}}


string DubstepFunction::run(FunctionArguments fargs) { // {{{
	return "WUB WUB WUB";
} // }}}
string DubstepFunction::name() const { // {{{
	return "dubstep";
} // }}}
string DubstepFunction::help() const { // {{{
	return "Takes no arguments; rocks.";
} // }}}
string DubstepFunction::regex() const { // {{{
	return "^(!dubstep|WUB|wub|DUBSTEP|dubstep)( .*)?";
} // }}}


string OrFunction::run(FunctionArguments fargs) { // {{{
	if(!fargs.toUs)
		return "";
	queue<string> q;
	q.push(fargs.matches[1]);
	q.push(fargs.matches[2]);

	vector<string> results;
	boost::regex r(this->regex(), regex::perl);
	smatch matches;
	while(!q.empty()) {
		string cur = q.front(); q.pop();
		if(regex_match(cur, matches, r)) {
			q.push(matches[1]);
			q.push(matches[2]);
		} else {
			results.push_back(cur);
		}
	}

	return results[rand() % results.size()];
} // }}}
string OrFunction::name() const { // {{{
	return "or";
} // }}}
string OrFunction::help() const { // {{{
	return "Returns one of multiple possibilities";
} // }}}
string OrFunction::regex() const { // {{{
	return "(.*)\\s+or\\s+(.*)";
} // }}}


YesFunction::YesFunction(string nick) : m_nick(nick) { // {{{
} // }}}
string YesFunction::run(FunctionArguments fargs) { // {{{
	return "yes";
} // }}}
string YesFunction::name() const { // {{{
	return this->m_nick;
} // }}}
string YesFunction::help() const { // {{{
	return "Say yes when my name is said";
} // }}}
string YesFunction::regex() const { // {{{
	return (string)".*" + this->m_nick + ".*";
} // }}}

