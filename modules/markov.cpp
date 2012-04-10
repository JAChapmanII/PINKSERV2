#include "markov.hpp"
using std::ostream;
using std::istream;
using global::ChatLine;
using boost::smatch;

#include <random>
using std::uniform_real_distribution;
using std::generate_canonical;

#include <map>
using std::map;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <utility>
using std::pair;
using std::make_pair;

#include <sstream>
using std::stringstream;

#include <iostream>
using std::endl;

#include "config.hpp"
#include "util.hpp"
using util::split;
using util::join;
using util::subvector;
using util::last;
using util::contains;
using util::trim;
using util::filter;

#include "brain.hpp"

// Only variables used to store the chain information
static const unsigned markovOrder = 2;
static map<string, vector<pair<string, unsigned>>> markovModel;
// TODO: uhm, hmm?
static string joinSeparator = " ";
static vector<double> coefficientTable = { 1.0/16, 1.0/4, 1.0 };

template<typename T>
		void increment(vector<pair<string, T>> &l, string str, T n = 1);
template<typename T>
		T sum(vector<pair<string, T>> &l);
template<typename T>
		T value(vector<pair<string, T>> &l, string str);
void insert(string text);
void push(vector<string> words, unsigned order);
string fetch(vector<string> words);
string recover(string initial);
string count(string initial);
unsigned occurrences(string seed);

template<typename T> // {{{
		void increment(vector<pair<string, T>> &l, string str, T n) {
	for(auto i : l) {
		if(i.first == str) {
			i.second += n;
			return;
		}
	}
	l.push_back(make_pair(str, n));
} // }}}
template<typename T> // {{{
		T sum(vector<pair<string, T>> &l) {
	T s = 0;
	for(auto i : l)
		s += i.second;
	return s;
} // }}}
template<typename T> // {{{
		T value(vector<pair<string, T>> &l, string str) {
	for(auto i : l)
		if(i.first == str)
			return i.second;
	return 0;
} // }}}

// insert each possible map for a set of words using a specific order
void push(vector<string> words, unsigned order) { // {{{
	if(words.size() < order)
		return;
	// special case the 0th order chain
	if(order == 0) {
		for(auto w : words)
			increment(markovModel[""], w);
		return;
	}
	// insert first sets of chains
	for(unsigned s = 0; s < words.size() - order; ++s) {
		string chain = join(subvector(words, s, order), joinSeparator);
		string target = words[s + order];
		increment(markovModel[chain], target);
	}
} // }}}

// simple way to push all possible orders of splits for a string
void insert(string text) { // {{{
	vector<string> words = split(text);
	if(words.empty())
		return;
	for(unsigned o = 0; o <= markovOrder; ++o)
		push(words, o);
} // }}}

// return a random endpoint given a seed
string fetch(vector<string> seed) { // {{{
	vector<pair<string, double>> ends;
	for(unsigned i = 1; (i <= seed.size()) && (i <= markovOrder); ++i) {
		vector<pair<string, unsigned>> orderI =
			markovModel[join(last(seed, i), joinSeparator)];
		for(auto j : orderI)
			increment(ends, j.first, j.second * coefficientTable[i - 1]);
	}

	// if we have nothing about that seed, return nothing
	if(ends.empty())
		return "";

	// add up total occurences of all end points
	double total = sum(ends);

	// pick a random number in [0, total)
	uniform_real_distribution<> urd(0, total);
	double r = urd(global::rengine);

	// find the end point corresponding to that
	auto i = ends.begin();
	for(; (i != ends.end()) && (r >= i->second); ++i)
		r -= i->second;
	if(i == ends.end()) {
		global::err << "markov::fetch: oh shit ran off the end of ends!" << endl;
		return "";
	}
	return i->first;
} // }}}

// Handles returning markov chains by calling fetch repeatedly
string recover(string initial) { // {{{
	vector<string> chain = split(initial);
	unsigned initialSize = chain.size();

	bool done = false;
	while(!done) {
		vector<string> seed;
		// create the current seed
		if(chain.size() < markovOrder)
			seed = chain;
		else
			seed = last(chain, markovOrder);

		// find a random endpoint
		string next = fetch(seed);

		// if it is empty, it's because we don't know anything about that
		if(next.empty())
			break;

		// add next to the string
		chain.push_back(next);

		// check to see if we should try to pick another one or just be done
		// the goal here is to make it more likely to fetch again when the
		// string is small
		double prob = 5.0 / (chain.size() - initialSize);

		// if we haven't even reach chain length yet, make it twice as
		// unlikely that we don't continue
		if(chain.size() <= initialSize * 1.5)
			prob += (1 - prob) / 2.0;

		// cap the probability at something somewhat sensible
		// 	aim for 50% chance to get to 12 words if it was capped each time
		if(prob > .945)
			prob = .945;

		// if we're currently ending with a punctuation, greatly increase the
		// chance of ending and sounding somewhat coherent
		if(((string)".?!;").find(next[next.length() - 1]) != string::npos)
			prob /= 1.85;

		// if a random num in [0, 1] is above our probability of ending, end
		if(generate_canonical<double, 16>(global::rengine) > prob)
			done = true;
	}

	// return the generated string
	return join(chain, " ");
} // }}}

// count the occurrences of a seed
unsigned occurrences(string seed) { // {{{
	if(!contains(markovModel, seed))
		return 0;
	return sum(markovModel[seed]);
} // }}}

// list chain count
string count(string initial) { // {{{
	vector<string> chain = split(initial);

	string seed;
	if(chain.size() < markovOrder)
		seed = join(chain, joinSeparator);
	else
		seed = join(last(chain, markovOrder), joinSeparator);

	// count occurences of the seed string
	unsigned total = occurrences(seed);

	// count total endpoints for all seeds
	unsigned long totalEnds = 0;
	for(auto i : markovModel)
		totalEnds += i.second.size();

	stringstream ss;
	ss << "Chains starting with: " << seed << ": ("
		// total unique endpoints for this seed, total occurences of this seed
		<< markovModel[seed].size() << ", " << total << ") ["
		// along with the total start points in the markov model
		<< markovModel.size() << ", "
		// finishing with total endpoints for all seeds
		<< totalEnds << "]";

	return ss.str();
} // }}}

MarkovFunction::MarkovFunction() : Function(true) { // {{{
} // }}}
string MarkovFunction::run(ChatLine line, smatch matches) { // {{{
	string seed = matches[1], r = recover(seed);
	if(r == seed)
		return "Sorry, I don't know anything about that";
	return r;
} // }}}
std::string MarkovFunction::passive(global::ChatLine line, bool parsed) { // {{{
	if(!parsed && !line.text.empty())
		insert(line.text);
	if(generate_canonical<double, 16>(global::rengine) <
			config::markovResponseChance) {
		string res = recover(line.text);
		if(res != line.text)
			return res;
	}
	return "";
} // }}}
string MarkovFunction::name() const { // {{{
	return "markov";
} // }}}
string MarkovFunction::help() const { // {{{
	return "Returns a markov chain.";
} // }}}
string MarkovFunction::regex() const { // {{{
	return "^!markov\\s+(.*)";
} // }}}
ostream &MarkovFunction::output(ostream &out) { // {{{
	return brain::write(out, markovModel);
} // }}}
istream &MarkovFunction::input(istream &in) { // {{{
	return brain::read(in, markovModel);
} // }}}


string ChainCountFunction::run(ChatLine line, smatch matches) { // {{{
	return count(matches[1]);
} // }}}
string ChainCountFunction::name() const { // {{{
	return "ccount";
} // }}}
string ChainCountFunction::help() const { // {{{
	return "Return number of markov chains";
} // }}}
string ChainCountFunction::regex() const { // {{{
	return "^!c+ount\\s+(.+)";
} // }}}


string CorrectionFunction::run(ChatLine line, smatch matches) { // {{{
	for(auto l = global::lastLog.rbegin(); l != global::lastLog.rend(); ++l) {
		string cline = this->correct(l->text);
		if(!cline.empty())
			return line.nick + ": maybe they meant " + cline;
		break;
	}
	return line.nick + ": nothing irregular found, sorry";
} // }}}
string CorrectionFunction::passive(global::ChatLine line, bool parsed) { // {{{
	if(parsed)
		return "";
	if(generate_canonical<double, 16>(global::rengine) <
			config::correctionResponseChance) {
		string cline = this->correct(line.text);
		if(cline.empty())
			return "";
		return line.nick + ": did you mean " + cline;
	}
	return "";
} // }}}
string CorrectionFunction::name() const { // {{{
	return "correct";
} // }}}
string CorrectionFunction::help() const { // {{{
	return "Magically corrects you";
} // }}}
string CorrectionFunction::regex() const { // {{{
	return "^!correct(\\s+.*)?";
} // }}}
string CorrectionFunction::correct(string line) { // {{{
	vector<string> words = split(line);
	words = filter(words, [](string s){ return !s.empty(); });
	if(words.size() < markovOrder + 1)
		return "";
	global::log << "----- attempting to correct: " << line << endl;
	string prefix = "";
	unsigned last = words.size() - (markovOrder + 1);
	for(unsigned i = 0; i < last; prefix += words[i] + " ", ++i) {
		vector<string> currentPhrase = subvector(words, i, markovOrder);
		string seed = join(currentPhrase, joinSeparator),
				target = words[i + markovOrder];

		unsigned ocount = occurrences(seed);
		if(ocount == 0)
			continue;

		double p = 0.0, ap = 1.0/ocount;
		if(contains(markovModel[seed], target))
			p = value(markovModel[seed], target) / (double)ocount;

		global::log << "seed: \"" << seed << "\","
			<< " target: \"" << target << "\", "
			<< "p, ap: " << p << ", " << ap << endl;

		if((ap > 0) && (p < ap * .60)) {
			string res = trim(prefix);
			if(!prefix.empty())
				res += " ";
			res += recover(join(currentPhrase, joinSeparator));
			if(((string)".?;,:").find(res[res.length() - 1]) == string::npos)
				res += "?";
			return res;
		}
	}
	return "";
} // }}}


