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

#include <iostream>
using std::endl;

#include "config.hpp"
#include "util.hpp"
using util::split;
using util::join;
using util::subvector;
using util::contains;
using util::trim;
using util::filter;

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
unsigned occurrences(string seed);
double probability(string seed, string end);

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

// count the occurrences of a seed
unsigned occurrences(string seed) { // {{{
	if(!contains(markovModel, seed))
		return 0;
	map<string, unsigned> seedMap = markovModel[seed];
	unsigned total = 0;
	for(auto i : seedMap)
		total += i.second;
	return total;
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

// determine the probability that a chain would occur
double probability(string seed, string end) { // {{{
	if(!contains(markovModel, seed) || !contains(markovModel[seed], end))
		return 0;
	return (double)markovModel[seed][end] / occurrences(seed);
} // }}}

string MarkovFunction::run(FunctionArguments fargs) { // {{{
	return recover(fargs.matches[1]);
} // }}}
std::string MarkovFunction::passive(global::ChatLine line, bool parsed) { // {{{
	if(!parsed && !line.text.empty())
		insert(line.text);
	double r = (double)rand() / RAND_MAX;
	if(r < config::markovResponseChance) {
		string res = recover(line.text);
		if((res != line.text) && !unknownSeed)
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


string CorrectionFunction::passive(global::ChatLine line, bool parsed) { // {{{
	if(parsed)
		return "";
	double r = (double)rand() / RAND_MAX;
	if(line.nick == "JDLJDL")
		r /= 4.0;
	if(r < config::correctionResponseChance) {
		vector<string> words = split(line.text);
		words = filter(words, [](string s){ return !s.empty(); });
		if(words.size() < markovOrder + 1)
			return "";
		global::log << "------------------------------------------------" << endl;
		string prefix = "";
		unsigned last = words.size() - (markovOrder + 1);
		for(unsigned i = 0; i < last; prefix += words[i] + " ", ++i) {
			vector<string> currentPhrase = subvector(words, i, markovOrder);
			string seed = join(currentPhrase), target = words[i + markovOrder];

			unsigned ocount = occurrences(seed);
			if(ocount == 0)
				continue;

			double p = 0.0, ap = 1.0/ocount;
			if(contains(markovModel[seed], target))
				p = (double)markovModel[seed][target] / ocount;

			global::log << "seed: \"" << seed << "\","
				<< " target: \"" << target << "\", "
				<< "p, ap: " << p << ", " << ap << endl;

			if(line.nick == "JDLJDL")
				p *= 2.0, ap /= 2.0;

			if((ap > 0) && (p < ap * .60)) {
				string res = line.nick + ": did you mean " + trim(prefix);
				if(!prefix.empty())
					res += " ";
				res += recover(join(currentPhrase, " "));
				if(((string)".?;,:").find(res[res.length() - 1]) == string::npos)
					res += "?";

				return res;
			}
		}
	}
	return "";
} // }}}
string CorrectionFunction::name() const { // {{{
	return "correct";
} // }}}
string CorrectionFunction::help() const { // {{{
	return "Magically corrects you";
} // }}}


