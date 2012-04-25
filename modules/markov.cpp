#include "markov.hpp"
using std::ostream;
using std::istream;
using global::ChatLine;
using boost::smatch;

#include <random>
using std::uniform_real_distribution;
using std::uniform_int_distribution;
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

#include <queue>
using std::queue;

#include "config.hpp"
#include "util.hpp"
using util::split;
using util::join;
using util::subvector;
using util::last;
using util::contains;
using util::trim;
using util::filter;
using util::asString;

#include "brain.hpp"
#include "dictionary.hpp"

template<int O> class MarkovModel { // {{{
	public:
		MarkovModel() : m_order(O), m_model() { }
		void increment(queue<string> chain, string target, unsigned count = 1);
		string random(queue<string> chain);

		bool contains(queue<string> chain);
		unsigned operator[](queue<string> chain);
		MarkovModel<O - 1> operator[](string word);
		unsigned size() const;

		ostream &write(ostream &out);
		istream &read(istream &in);

		map<unsigned, unsigned> endpoint(queue<string> chain);
		unsigned total(queue<string> chain);

	protected:
		unsigned m_order;
		map<unsigned, MarkovModel<O - 1>> m_model;
}; // }}}

template<int O> void MarkovModel<O>::increment( // {{{
		queue<string> chain, string target, unsigned count) {
	// if there are two many words in the prefix, pop them off
	while(chain.size() > this->m_order)
		chain.pop();

	// determine the start point id
	unsigned v = dictionary.fetch("");
	if(chain.size() == this->m_order)
		v = dictionary.fetch(chain.front());

	// insert the chain in the appropriate lower order model
	this->m_model[v].increment(chain, target, count);
} // }}}
template<int O> string MarkovModel<O>::random(queue<string> chain) { // {{{
	// if there are two many words in the prefix, pop them off
	while(chain.size() > this->m_order)
		chain.pop();

	// determine the start point id
	unsigned v = dictionary.fetch("");
	if(chain.size() == this->m_order)
		v = dictionary.fetch(chain.front());

	// fetch random from the appropriate lower order model
	return this->m_model[v].random(chain);
} // }}}
template<int O> bool MarkovModel<O>::contains(queue<string> chain) { // {{{
	// if there are two many words in the prefix, pop them off
	while(chain.size() > this->m_order)
		chain.pop();

	// determine the start point id
	unsigned v = dictionary.fetch("");
	if(chain.size() == this->m_order)
		v = dictionary.fetch(chain.front());

	// if we haven't seen this seed at this level, we can't see it below
	if(!util::contains(this->m_model, v))
		return false;

	// find out if the lower order model has seen this seed
	return this->m_model[v].contains(chain);
} // }}}
template<int O> unsigned MarkovModel<O>::operator[](queue<string> chain) { // {{{
	// if there are two many words in the prefix, pop them off
	while(chain.size() > this->m_order)
		chain.pop();

	// determine the start point id
	unsigned v = dictionary.fetch("");
	if(chain.size() == this->m_order)
		v = dictionary.fetch(chain.front());

	// fetch value from the appropriate lower order model
	return this->m_model[v][chain];
} // }}}
template<int O> MarkovModel<O - 1> MarkovModel<O>::operator[](string word) { // {{{
	return this->m_model[dictionary.fetch(word)];
} // }}}
template<int O> unsigned MarkovModel<O>::size() const { // {{{
	return this->m_model.size();
} // }}}
template<int O> ostream &MarkovModel<O>::write(ostream &out) { // {{{
	unsigned s = this->m_model.size();
	brain::write(out, s);
	for(auto i : this->m_model) {
		unsigned f = i.first;
		brain::write(out, f);
		i.second.write(out);
	}
	return out;
} // }}}
template<int O> istream &MarkovModel<O>::read(istream &in) { // {{{
	unsigned s = 0;
	brain::read(in, s);
	for(unsigned i = 0; i < s; ++i) {
		unsigned key;
		MarkovModel<O - 1> value;
		brain::read(in, key);
		value.read(in);
		this->m_model[key] = value;
	}
	return in;
} // }}}
template<int O> map<unsigned, unsigned> MarkovModel<O>::endpoint( // {{{
		queue<string> chain) {
	// if there are two many words in the prefix, pop them off
	while(chain.size() > this->m_order)
		chain.pop();

	// determine the start point id
	unsigned v = dictionary.fetch("");
	if(chain.size() == this->m_order)
		v = dictionary.fetch(chain.front());

	// fetch value from the appropriate lower order model
	return this->m_model[v].endpoint(chain);
} // }}}
template<int O> unsigned MarkovModel<O>::total(queue<string> chain) { // {{{
	// if there are two many words in the prefix, pop them off
	while(chain.size() > this->m_order)
		chain.pop();

	// determine the start point id
	unsigned v = dictionary.fetch("");
	if(chain.size() == this->m_order)
		v = dictionary.fetch(chain.front());

	// fetch value from the appropriate lower order model
	return this->m_model[v].total(chain);
} // }}}

template<> class MarkovModel<0> { // {{{
	public:
		MarkovModel() : m_model(), m_total(0) { }
		void increment(queue<string> chain, string target, unsigned count = 1);
		string random(queue<string> chain);

		bool contains(queue<string> chain);
		unsigned operator[](queue<string> chain);
		unsigned size() const;

		ostream &write(ostream &out);
		istream &read(istream &in);

		map<unsigned, unsigned> endpoint(queue<string> chain);
		unsigned total(queue<string> chain);

	protected:
		map<unsigned, unsigned> m_model;
		unsigned m_total;
}; // }}}

void MarkovModel<0>::increment( // {{{
		queue<string> chain, string target, unsigned count) {
	this->m_model[dictionary.fetch(target)] += count;
	this->m_total += count;
} // }}}
string MarkovModel<0>::random(queue<string> chain) { // {{{
	if(this->m_total == 0) {
		std::cerr << "this map is empty!?" << std::endl;
		return "";
	}
	uniform_int_distribution<> uid(1, this->m_total);
	unsigned target = uid(global::rengine);
	auto i = this->m_model.begin();
	for(; (i != this->m_model.end()) && (target > i->second); ++i)
		target -= i->second;
	if(i == this->m_model.end()) {
		std::cerr << "fell off the bandwagon" << endl;
		std::cerr << "total: " << this->m_total << endl;
		return "";
	}
	return dictionary.fetch(i->first);
} // }}}
bool MarkovModel<0>::contains(queue<string> chain) { // {{{
	if(chain.empty())
		return false;
	return util::contains(this->m_model, dictionary.fetch(chain.back()));
} // }}}
unsigned MarkovModel<0>::operator[](queue<string> chain) { // {{{
	if(chain.empty())
		return 0;
	return this->m_model[dictionary.fetch(chain.back())];
} // }}}
unsigned MarkovModel<0>::size() const { // {{{
	return this->m_model.size();
} // }}}
ostream &MarkovModel<0>::write(ostream &out) { // {{{
	return brain::write(out, this->m_model);
} // }}}
istream &MarkovModel<0>::read(istream &in) { // {{{
	brain::read(in, this->m_model);
	for(auto i : this->m_model)
		this->m_total += i.second;
	return in;
} // }}}
map<unsigned, unsigned> MarkovModel<0>::endpoint(queue<string> chain) { // {{{
	return this->m_model;
} // }}}
unsigned MarkovModel<0>::total(queue<string> chain) {
	return this->m_total;
}

static const unsigned markovOrder = 2;
static MarkovModel<markovOrder> markovModel;

// TODO: uhm, hmm?
static string joinSeparator = " ";
static vector<double> coefficientTable = { 1.0/36, 1.0/6, 1.0 };

template<typename T>
		T sum(vector<pair<string, T>> &l);
template<typename T>
		T value(vector<pair<string, T>> &l, string str);
void insert(string text);
void push(vector<string> words, unsigned order);
string fetch(vector<string> words);
string recover(string initial);
string count(string initial);
unsigned occurrences(vector<string> seed);

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
	queue<string> chain;
	for(auto i : seed)
		chain.push(i);
	while(chain.size() > markovOrder)
		chain.pop();

	map<unsigned, double> smoothModel;
	double smoothModelTotal = 0;
	while(!chain.empty()) {
		map<unsigned, unsigned> cmodel = markovModel.endpoint(chain);
		for(auto i : cmodel) {
			smoothModel[i.first] += coefficientTable[chain.size()];
			smoothModelTotal += coefficientTable[chain.size()];
		}
		chain.pop();
	}

	// if we have nothing about that seed, return nothing
	if(smoothModel.empty())
		return "";

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
	return dictionary.fetch(i->first);
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
unsigned occurrences(vector<string> seed) { // {{{
	queue<string> chain;
	for(auto i : seed)
		chain.push(i);
	return markovModel.total(chain);
} // }}}

// list chain count
string count(string initial) { // {{{
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
		string res = recover(join(last(split(line.text), markovOrder + 1), " "));
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
	return "^!markov\\s+(.+)";
} // }}}
ostream &MarkovFunction::output(ostream &out) { // {{{
	markovModel.write(out);
	return dictionary.write(out);
} // }}}
istream &MarkovFunction::input(istream &in) { // {{{
	markovModel.read(in);
	return dictionary.read(in);
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
			return res;
		}
	}
	return "";
} // }}}


string DictionarySizeFunction::run(ChatLine line, smatch matches) { // {{{
	return line.nick + ": " + asString(dictionary.size());
} // }}}
string DictionarySizeFunction::name() const { // {{{
	return "dsize";
} // }}}
string DictionarySizeFunction::help() const { // {{{
	return "Return number of unique 1-grams";
} // }}}
string DictionarySizeFunction::regex() const { // {{{
	return "^!dsize(\\s+.*)?";
} // }}}


