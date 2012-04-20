#include "simple.hpp"
using std::string;
using boost::smatch;
using global::ChatLine;

#include <queue>
using std::queue;

#include <vector>
using std::vector;

#include <random>
using std::uniform_int_distribution;
using std::bernoulli_distribution;

#include <boost/regex.hpp>
using boost::regex;

string WaveFunction::run(ChatLine line, smatch matches) { // {{{
	string wave = matches[1];
	if(wave[0] == 'o')
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
	return ".*?(o/|\\\\o)( .*)?";
} // }}}


string LoveFunction::run(ChatLine line, smatch matches) { // {{{
	string slash = matches[1];
	if(!slash.empty())
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
	return "^<(/?)3( .*)?";
} // }}}


string FishFunction::run(ChatLine line, smatch matches) { // {{{
	int fcount = 1;
	string es = matches[1];
	if(!es.empty()) {
		uniform_int_distribution<> uid(2, 7);
		fcount = uid(global::rengine);
	}

	string fishies;
	bernoulli_distribution d(0.50);
	uniform_int_distribution<> uid(1, 3);
	for(int i = 0; i < fcount; ++i) {
		if(d(global::rengine))
			fishies += "<><";
		else
			fishies += "><>";
		fishies += string(uid(global::rengine), ' ');
	}
	return fishies;
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


string TrainFunction::run(ChatLine line, smatch matches) { // {{{
	string extra = matches[1];
	uniform_int_distribution<> carCount(0, 8);
	int ccount = carCount(global::rengine);
	bernoulli_distribution d(0.50);
	bool dir = d(global::rengine);
	string train;
	if(dir)
		train += "/.==.]";
	else
		train += "{. .}";
	for(int i = 0; i < ccount; ++i)
		train += "[. .]";
	if(dir)
		train += "{. .}";
	else
		train += "[.==.\\";

	return train;
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


string DubstepFunction::run(ChatLine line, smatch matches) { // {{{
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


string OrFunction::run(ChatLine line, smatch matches) { // {{{
	if(!line.toUs)
		return "";
	queue<string> q;
	q.push(matches[1]);
	q.push(matches[2]);

	vector<string> results;
	boost::regex r(this->regex(), regex::perl);
	while(!q.empty()) {
		string cur = q.front(); q.pop();
		if(regex_match(cur, matches, r)) {
			q.push(matches[1]);
			q.push(matches[2]);
		} else {
			results.push_back(cur);
		}
	}

	uniform_int_distribution<> uid(0, results.size() - 1);
	return results[uid(global::rengine)];
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
string YesFunction::run(ChatLine line, smatch matches) { // {{{
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


string SayFunction::run(ChatLine line, smatch matches) { // {{{
	return matches[1];
} // }}}
string SayFunction::name() const { // {{{
	return "say";
} // }}}
string SayFunction::help() const { // {{{
	return "Say something";
} // }}}
string SayFunction::regex() const { // {{{
	return "^!say\\s(.+)";
} // }}}

string TellFunction::run(ChatLine line, smatch matches) { // {{{
	global::send(matches[1], matches[2], true);
	return "";
} // }}}
string TellFunction::name() const { // {{{
	return "tell";
} // }}}
string TellFunction::help() const { // {{{
	return "Tell someone something";
} // }}}
string TellFunction::regex() const { // {{{
	return "^!tell\\s+(.+)\\s+(.+)";
} // }}}

