#include "simple.hpp"
using std::string;
using boost::smatch;

#include <queue>
using std::queue;

#include <vector>
using std::vector;

#include <random>
using std::uniform_int_distribution;
using std::bernoulli_distribution;

#include <boost/regex.hpp>
using boost::regex;

#include "global.hpp"

WaveFunction::WaveFunction() : Function( // {{{
		"wave", "Takes no arguments; waves.", ".*?(o/|\\\\o)( .*)?") {
} // }}}
string WaveFunction::run(ChatLine line, smatch matches) { // {{{
	string wave = matches[1];
	if(wave[0] == 'o')
		return "\\o";
	else
		return "o/";
} // }}}


LoveFunction::LoveFunction() : Function( // {{{
		"</?3", "Takes no arguments; outputs love.", "^<(/?)3( .*)?") {
} // }}}
string LoveFunction::run(ChatLine line, smatch matches) { // {{{
	string slash = matches[1];
	if(!slash.empty())
		return ":(";
	return "<3";
} // }}}


FishFunction::FishFunction() : Function( // {{{
		"fish(es)?", "Takes no arguments; outputs fish(es).",
		"^!fish(es)?( .*)?") {
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


TrainFunction::TrainFunction() : Function( // {{{
		"sl", "Takes no arguments; returns a train.", "^!sl( .*)?") {
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


DubstepFunction::DubstepFunction() : Function( // {{{
		"dubstep", "Takes no arguments; rocks.",
		"^(!dubstep|WUB|wub|DUBSTEP|dubstep)( .*)?") {
} // }}}
string DubstepFunction::run(ChatLine line, smatch matches) { // {{{
	return "WUB WUB WUB";
} // }}}


OrFunction::OrFunction() : Function( // {{{
		"or", "Returns one of multiple possibilities", "(.*)\\s+or\\s+(.*)") {
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


YesFunction::YesFunction(string nick) : Function( // {{{
		nick, "Say yes when my name is said", ".*" + nick + ".*") {
} // }}}
string YesFunction::run(ChatLine line, smatch matches) { // {{{
	return "yes";
} // }}}


SayFunction::SayFunction() : Function( // {{{
		"speak", "Say something", "^!speak\\s(.+)") {
} // }}}
string SayFunction::run(ChatLine line, smatch matches) { // {{{
	return matches[1];
} // }}}


TellFunction::TellFunction() : Function( // {{{
		"tell", "Tell someone something", "^!tell\\s+(\\S+)\\s+(.+)") {
} // }}}
string TellFunction::run(ChatLine line, smatch matches) { // {{{
	global::send(matches[1], matches[2], true);
	return "";
} // }}}

