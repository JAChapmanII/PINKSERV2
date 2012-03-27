#include "markov.hpp"
using std::ostream;
using std::istream;

#include <map>
using std::map;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <sstream>
using std::stringstream;

#include "util.hpp"
using util::split;
using util::join;
using util::subvector;

#include "brain.hpp"

// Only variables used to store the chain information
static const unsigned markovOrder = 2;
static map<string, map<string, unsigned>> markovModel;
static bool unknownSeed = false;

void insert(string text);
void push(vector<string> words, unsigned order);
string fetch(string seed);
string recover(string initial);
string count(string initial);

// insert each possible map for a set of words using a specific order
void push(vector<string> words, unsigned order) { // {{{
	if(words.size() < order)
		return;
	// insert first sets of chains
	for(unsigned s = 0; s < words.size() - order; ++s) {
		string chain = join(subvector(words, s, order));
		string target = words[s + order];
		markovModel[chain][target]++;
	}

	// insert last set of words -> null mapping
	string lchain = join(subvector(words, words.size() - order, order));
	markovModel[lchain][""]++;
} // }}}

// simple way to push all possible orders of splits for a string
void insert(string text) { // {{{
	vector<string> words = split(text);
	if(words.empty())
		return;
	for(unsigned o = 1; o <= markovOrder; ++o)
		push(words, o);
} // }}}

// return a random endpoint given a seed
string fetch(string seed) { // {{{
	unknownSeed = false;
	// if we have nothing about that seed, return nothing
	if(markovModel[seed].empty()) {
		unknownSeed = true;
		return "";
	}

	// add up total occurences of all end points
	unsigned total = 0;
	map<string, unsigned> seedMap = markovModel[seed];
	for(auto i : seedMap)
		total += i.second;

	// pick a random number in [0, total)
	unsigned r = rand() % total;

	// find the end point corresponding to that
	auto i = seedMap.begin();
	for(; r >= i->second; ++i)
		r -= i->second;
	return i->first;
} // }}}

// Handles returning markov chains by calling fetch repeatedly
string recover(string initial) { // {{{
	vector<string> chain = split(initial);

	bool done = false;
	while(!done) {
		string seed;
		// create the current seed
		if(chain.size() < markovOrder)
			seed = join(chain);
		else
			seed = join(subvector(chain, chain.size() - markovOrder, markovOrder));

		// find a random endpoint
		string next = fetch(seed);

		// if it is empty
		if(next.empty()) {
			// if it's because we don't know anything about that
			if(unknownSeed) {
				return "Sorry, I don't know anything about that";
			}
			// check to see if we should try to pick another one or just be done
			// the goal here is to make it more likely to fetch again when the
			// string is small
			double prob = pow(0.5, chain.size());

			// if we haven't even reach chain length yet, make it twice as
			// unlikely that we don't continue
			if(chain.size() < markovOrder)
				prob += (1 - prob) / 2.0;

			// if a random num in [0, 1] is above our probability of ending, end
			if(((double)rand() / RAND_MAX) > prob)
				done = true;
		} else {
			// otherwise add it to the string
			chain.push_back(next);
		}
	}

	// return the generated string
	return join(chain, " ");
} // }}}

// list chain count
string count(string initial) { // {{{
	vector<string> chain = split(initial);

	string seed;
	if(chain.size() < markovOrder)
		seed = join(chain);
	else
		seed = join(subvector(chain, chain.size() - markovOrder, markovOrder));

	// count occurences of the seed string
	map<string, unsigned> seedMap = markovModel[seed];
	unsigned total = 0;
	for(auto i : seedMap)
		total += i.second;

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

string MarkovFunction::run(FunctionArguments fargs) { // {{{
	return recover(fargs.matches[1]);
} // }}}
void MarkovFunction::passive(global::ChatLine line, bool parsed) { // {{{
	if(!parsed && !line.text.empty())
		insert(line.text);
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


string ChainCountFunction::run(FunctionArguments fargs) { // {{{
	return count(fargs.matches[1]);
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

