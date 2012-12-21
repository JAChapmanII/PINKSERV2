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


TellFunction::TellFunction() : Function( // {{{
		"tell", "Tell someone something", "^!tell\\s+(\\S+)\\s+(.+)") {
} // }}}
string TellFunction::run(ChatLine line, smatch matches) { // {{{
	global::send(matches[1], matches[2], true);
	return "";
} // }}}

