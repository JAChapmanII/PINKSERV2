#include "markov.hpp"
using std::ostream;
using std::istream;
using std::vector;
using std::string;

#include <random>
using std::uniform_real_distribution;
using std::uniform_int_distribution;
using std::generate_canonical;

#include <map>
using std::map;

#include <utility>
using std::pair;
using std::make_pair;

#include <sstream>
using std::stringstream;

#include <iostream>
using std::endl;

#include <queue>
using std::queue;

#include <algorithm>
using std::remove_if;

#include "config.hpp"
#include "util.hpp"
using util::split;
using util::join;
using util::subvector;
using util::last;
using util::contains;
using util::trim;
using util::asString;
using util::fromString;

#include "brain.hpp"
#include "global.hpp"
using global::dictionary;

#include "markovmodel.hpp"

static const unsigned markovOrder = 2;
static MarkovModel<markovOrder> markovModel;

// TODO: uhm, hmm?
static string joinSeparator = " ";
// TODO: best values for these?
// TODO: adjustable? that would be cool :D same with order?
static vector<double> coefficientTable = { 1.0/128.0, 1.0/8.0, 1.0 };

// TODO: move these into util?
template<typename T> T sum(vector<pair<string, T>> &l);
template<typename T> T value(vector<pair<string, T>> &l, string str);

// insert each possible map for a set of words using a specific order
void insert(string text);
// simple way to push all possible orders of splits for a string
void push(vector<string> words, unsigned order);

// return a random endpoint given a seed
string fetch(vector<string> words);
// Handles returning markov chains by calling fetch repeatedly
string recover(string initial);

// list chain count
string count(string initial);
// count the occurrences of a seed
unsigned occurrences(vector<string> seed);


template<typename T>
		T sum(vector<pair<string, T>> &l) {
	T s = 0;
	for(auto i : l)
		s += i.second;
	return s;
}
template<typename T>
		T value(vector<pair<string, T>> &l, string str) {
	for(auto i : l)
		if(i.first == str)
			return i.second;
	return 0;
}

void push(vector<string> words, unsigned order) {
	if(words.size() <= order)
		return;
	// special case the 0th order chain
	if(order == 0) {
		queue<string> empty;
		for(auto w : words)
			markovModel.increment(empty, w);
		return;
	}

	// insert first chain (build word queue first)
	queue<string> chain;
	for(unsigned i = 0; i < order; ++i)
		chain.push(words[i]);
	markovModel.increment(chain, words[order]);

	// insert the remaining chains
	for(unsigned e = order + 1; e < words.size(); ++e) {
		chain.pop();
		chain.push(words[e - 1]);
		markovModel.increment(chain, words[e]);
	}
}
void insert(string text) {
	vector<string> words = split(text);
	if(words.empty())
		return;
	for(unsigned o = 0; o <= markovOrder; ++o)
		push(words, o);
}

string fetch(vector<string> seed) {
	queue<string> chain;
	for(auto i : seed)
		chain.push(i);
	while(chain.size() > markovOrder)
		chain.pop();

	map<unsigned, double> smoothModel;
	double smoothModelTotal = 0;
	double weight = 0;
	while(!chain.empty()) {
		map<unsigned, unsigned> cmodel = markovModel.endpoint(chain);
		for(auto i : cmodel) {
			weight = coefficientTable[chain.size()] * i.second / cmodel.size();
			// TODO: see how the division here works out
			smoothModel[i.first] += weight;
			smoothModelTotal += weight;
		}
		chain.pop();
	}

	// if we have nothing about that seed, return nothing
	if(smoothModel.empty())
		return "";

	// add 0th order model
	map<unsigned, unsigned> model0 = markovModel.endpoint(chain);
	for(auto i : model0) {
		weight = coefficientTable[0] * i.second / model0.size();
		smoothModel[i.first] += weight;
		smoothModelTotal += weight;
	}

	// pick a random number in [0, total)
	uniform_real_distribution<> urd(0, smoothModelTotal);
	double r = urd(global::rengine);

	// find the end point corresponding to that
	auto i = smoothModel.begin();
	for(; (i != smoothModel.end()) && (r >= i->second); ++i)
		r -= i->second;
	if(i == smoothModel.end()) {
		global::err << "markov::fetch: oh shit ran off the end of ends!" << endl;
		return "";
	}
	return dictionary[i->first];
}
string recover(string initial) {
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
}

string count(string initial) {
	vector<string> chain = split(initial);

	string seed;
	if(chain.size() < markovOrder)
		seed = join(chain, joinSeparator);
	else
		seed = join(last(chain, markovOrder), joinSeparator);

	vector<string> vseed = split(seed);
	// count occurences of the seed string
	unsigned total = occurrences(vseed);

	// count total endpoints for all seeds
	unsigned long totalEnds = 0;
	//for(auto i : markovModel)
		//totalEnds += i.second.size();

	stringstream ss;
	ss << "Chains starting with: " << seed << ": ("
		// total unique endpoints for this seed, total occurences of this seed
		<< markovModel[seed].size() << ", " << total << ") ["
		// along with the total start points in the markov model
		<< markovModel.size() << ", "
		// finishing with total endpoints for all seeds
		<< totalEnds << "]";

	return ss.str();
}
unsigned occurrences(vector<string> seed) {
	queue<string> chain;
	for(auto i : seed)
		chain.push(i);
	return markovModel.total(chain);
}


// TODO: we're ignoring the return here...
void markovLoad(istream &in) {
	markovModel.read(in);
}
void markovSave(ostream &out) {
	markovModel.write(out);
}

Variable observe(vector<Variable> arguments) {
	insert(join(arguments, " "));
	return Variable(true, Permissions());
}

Variable markov(vector<Variable> arguments) {
	string seed = join(arguments, " "), r = recover(seed);
	if(r == seed)
		return Variable("Sorry, I don't know anything about that", Permissions());
	return Variable(r, Permissions());
}
Variable correct(vector<Variable> arguments) {
	string line = join(arguments, " ");

	vector<string> words = split(line);
	words.erase(remove_if(words.begin(), words.end(),
				[](const string &s){ return !s.empty(); }), words.end());
	if(words.size() < markovOrder + 1)
		return Variable("", Permissions());

	// TODO: this needs some cleanup
	global::log << "----- attempting to correct: " << line << endl;
	string prefix = "";
	unsigned last = words.size() - (markovOrder + 1);
	for(unsigned i = 0; i < last; prefix += words[i] + " ", ++i) {
		vector<string> currentPhrase = subvector(words, i, markovOrder);
		string seed = join(currentPhrase, joinSeparator),
				target = words[i + markovOrder];

		unsigned ocount = occurrences(split(seed));
		if(ocount == 0)
			continue;

		double ap = 1.0/ocount, p = 0;
		// TODO: oh god, p is broken
		//value(markovModel[seed], target) / (double)ocount;

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
			return Variable(res, Permissions());
		}
	}
	return Variable("", Permissions());
}

Variable ccount(vector<Variable> arguments) {
	return Variable(count(join(arguments, " ")), Permissions());
}
Variable dsize(vector<Variable> arguments) {
	return Variable((long)dictionary.size(), Permissions());
}
Variable rword(vector<Variable> arguments) {
	// TODO: proper bail under new system?
	if(arguments.size() > 2)
		throw (string)"rword may only take two numeric endpoints";

	string mins, maxs;
	if(arguments.size() > 0)
		mins = arguments[0].toString();
	if(arguments.size() > 1)
		maxs = arguments[1].toString();

	// TODO: type check arguments? Let caller handle it?

	queue<string> chain;
	// get 0th order model
	map<unsigned, unsigned> model0 = markovModel.endpoint(chain);
	// TODO: already defined maybe?
	double totalWeight = 0;
	for(auto i : model0)
		totalWeight += i.second;

	double min = 0, max = 1;
	if(!mins.empty())
		min = fromString<double>(mins);
	if(!maxs.empty())
		max = fromString<double>(maxs);

	// pick a random number in [0, total)
	uniform_real_distribution<> urd(min, max * totalWeight);
	double r = urd(global::rengine);

	// TODO: must sort the model for the range restriction to work properly

	// find the end point corresponding to that
	auto i = model0.begin();
	for(; (i != model0.end()) && (r >= i->second); ++i)
		r -= i->second;
	if(i == model0.end()) {
		global::err << "markov::fetch: oh shit ran off the end of ends!" << endl;
		throw (string)"markov::fetch: oh shit ran off the end of ends!";
	}
	return Variable(dictionary[i->first], Permissions());
}

