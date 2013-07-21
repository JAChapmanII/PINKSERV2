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

ostream &dumpMarkov(ostream &out, MarkovModel *model, string prefix);
ostream &dumpMarkov(ostream &out) {
	list<unsigned> blank;
	return dumpMarkov(out, markovModel[blank], "");
}
ostream &dumpMarkov(ostream &out, MarkovModel *model, string prefix) {
	list<unsigned> blank;
	for(auto it : *model) {
		out << prefix << dictionary[it.first] << "  "
			<< it.second->count(blank) << endl;
		dumpMarkov(out, it.second, prefix + dictionary[it.first] + " ");
	}
	return out;
}

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
	// TODO: implement
	throw (string)"error: correct unimplemented";
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
}

