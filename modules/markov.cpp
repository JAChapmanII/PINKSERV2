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

#include <list>
using std::list;

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

// TODO: turn into pbrane variable
static const unsigned maxMarkovOrder = 3;
static MarkovModel markovModel;

// TODO: uhm, hmm?
static string joinSeparator = " ";

// insert each possible map for a set of words using a specific order
void insert(string text);
// simple way to push all possible orders of splits for a string
void push(vector<string> words, unsigned order);

// return a random endpoint given a seed
unsigned fetch(list<unsigned> words);
string fetch(vector<string> words);
// Handles returning markov chains by calling fetch repeatedly
string recover(string initial);

// list chain count
string count(string initial);

void push(vector<string> words, unsigned order) {
	if(words.size() <= order)
		return;
	// special case the 0th order chain
	if(order == 0) {
		for(auto w : words) {
			list<unsigned> solo;
			solo.push_back(dictionary[w]);
			markovModel.increment(solo);
		}
		return;
	}

	// insert first chain (build word queue first)
	list<unsigned> chain;
	for(unsigned i = 0; i <= order; ++i)
		chain.push_back(dictionary[words[i]]);
	markovModel.increment(chain);

	// insert the remaining chains
	for(unsigned e = order + 1; e < words.size(); ++e) {
		chain.pop_front();
		chain.push_back(dictionary[words[e]]);
		markovModel.increment(chain);
	}
}
void insert(string text) {
	vector<string> words = split(text);
	if(words.empty())
		return;
	for(unsigned o = 0; o <= maxMarkovOrder; ++o)
		push(words, o);
}

unsigned fetch(list<unsigned> words) {
	return markovModel.random(words);
}
string fetch(vector<string> seed) {
	list<unsigned> seedl;
	for(auto i : seed)
		seedl.push_back(dictionary[i]);
	return dictionary[fetch(seedl)];

	/*
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
	*/
}
string recover(string initial) {
	vector<string> ivec = split(initial);
	list<unsigned> chain;
	for(auto i : ivec)
		chain.push_back(dictionary[i]);

	bool done = false;
	while(!done) {
		// find a random endpoint
		unsigned next = fetch(chain);
		string nexts = dictionary[next];

		// if it is empty, it's because we don't know anything about that
		if(next == 0)
			break;

		// add next to the string
		chain.push_back(next);

		// check to see if we should try to pick another one or just be done
		double prob = 0.9;
		if(chain.size() <= maxMarkovOrder * 2.5)
			prob += (1 - prob) / 2.0;

		// if we're currently ending with a punctuation, greatly increase the
		// chance of ending and sounding somewhat coherent
		if(((string)".?!;").find(nexts.back()) != string::npos)
			prob /= 1.85;

		// if a random num in [0, 1] is above our probability of ending, end
		if(generate_canonical<double, 16>(global::rengine) > prob)
			done = true;
	}

	// return the generated string
	string result;
	for(auto i : chain)
		result += dictionary[i] + " ";
	result.pop_back();
	return result;
}

string count(string initial) {
	vector<string> seedv = split(initial);
	list<unsigned> seed;
	for(auto i : seedv)
		seed.push_back(dictionary[i]);

	// count occurences of the seed string
	unsigned total = markovModel.count(seed);

	stringstream ss;
	ss << "Chains starting with: " << initial << ": ("
		// total unique endpoints for this seed, total occurences of this seed
		<< markovModel[seed]->size() << ", " << total << ") ["
		// along with the total start points in the markov model
		<< markovModel.size() << "]";

	return ss.str();
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
	throw (string)"error: correct unimplemented";
	/*
	string line = join(arguments, " ");

	vector<string> words = split(line);
	words.erase(remove_if(words.begin(), words.end(),
				[](const string &s){ return !s.empty(); }), words.end());

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
	*/
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

	list<unsigned> chain;
	return Variable(dictionary[markovModel.random(chain)], Permissions());

	// TODO: reimplement
	/*
	// TODO: type check arguments? Let caller handle it?

	list<unsigned> chain;
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
	*/
}

