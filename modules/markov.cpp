#include "markov.hpp"

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

/*
	// load markov file if it exists {{{
	ifstream in(markovFileName);
	if(in.good()) {
		log << "reading markov chain entries" << endl;
		string fline;
		getline(in, fline);
		stringstream ss;
		ss << fline;
		unsigned lcount = 0, tcount = 0;
		ss >> tcount;
		log << "\tsupposed total count: " << tcount << endl;
		log << "\t";

		unsigned percent = 0;
		while(!in.eof()) {
			string line;
			getline(in, line);
			if(in.eof() || !in.good())
				break;
			insert(line);
			lcount++;
			if((double)lcount / tcount * 100 > percent) {
				if(percent % 10 == 0)
					log << percent;
				else
					log << ".";
				log.flush();
				percent++;
			}
		}
		log << endl << markovFileName << ": read " << lcount << " lines" << endl;
	}
	in.close();
	// }}}
*/

map<string, map<string, unsigned>> markovModel;
const unsigned markovOrder = 2;

void insert(string text);
void markov_push(vector<string> words, unsigned order);
string fetch(string seed);

void markov_push(vector<string> words, unsigned order) { // {{{
	if(words.size() < order)
		return;

	// insert first sets of chains
	for(unsigned s = 0; s < words.size() - order; ++s) {
		string start = words[s];
		for(unsigned i = 1; i < order; ++i)
			start += (string)" " + words[s + i];
		markovModel[start][words[s + order]]++;
	}

	// insert last few words -> null mapping
	string start = words[words.size() - order];
	for(unsigned i = 1; i < order; ++i)
		start += (string)" " + words[words.size() - order + i];
	markovModel[start][""]++;
} // }}}
void insert(string text) { // {{{
	vector<string> words = split(text, " \t");
	for(unsigned o = 1; o <= markovOrder; ++o)
		markov_push(words, o);
} // }}}
string fetch(string seed) { // {{{
	if(markovModel[seed].empty())
		return "";
	unsigned total = 0;
	map<string, unsigned> seedMap = markovModel[seed];
	for(auto i : seedMap)
		total += i.second;
	unsigned r = rand() % total;
	auto i = seedMap.begin();
	while(r > i->second) {
		r -= i->second;
		++i;
	}
	return i->first;
} // }}}


string MarkovFunction::run(FunctionArguments fargs) { // {{{
	string init = fargs.matches[1];
	vector<string> words = split(init, " \t");

	string start, next;
	do {
		start = "";
		if(words.size() < markovOrder) {
			start = words[0];
			for(unsigned i = 1; i < words.size(); ++i)
				start += (string)" " + words[i];
		} else {
			start = words[words.size() - markovOrder];
			for(unsigned i = 1; i < markovOrder; ++i)
				start += (string)" " + words[words.size() - markovOrder + i];
		}

		next = fetch(start);
		if(!next.empty())
			words.push_back(next);
	} while(!next.empty());

	stringstream chain;
	chain << words[0];
	for(unsigned i = 1; i < words.size(); ++i)
		chain << " " << words[i];
	return chain.str();
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


string ChainCountFunction::run(FunctionArguments fargs) { // {{{
	string init = fargs.matches[2], cs = fargs.matches[1];
	vector<string> words = split(init, " \t");

	string start = "";
	if(words.size() < markovOrder) {
		start = words[0];
		for(unsigned i = 1; i < words.size(); ++i)
			start += (string)" " + words[i];
	} else {
		start = words[words.size() - markovOrder];
		for(unsigned i = 1; i < markovOrder; ++i)
			start += (string)" " + words[words.size() - markovOrder + i];
	}

	map<string, unsigned> seedMap = markovModel[start];
	unsigned total = 0;
	for(auto i : seedMap)
		total += i.second;

	stringstream ss;
	ss << "Chains starting with: " << start << ": ("
		<< markovModel[start].size() << ", " << total << ") ["
		<< markovModel.size() << "]";

	if(cs.length() > 2) {
		unsigned long totalEnds = 0;
		for(auto i : markovModel)
			totalEnds += i.second.size();
		ss << " {" << totalEnds << "}";
	}

	return ss.str();
} // }}}
string ChainCountFunction::name() const { // {{{
	return "ccount";
} // }}}
string ChainCountFunction::help() const { // {{{
	return "Return number of markov chains";
} // }}}
string ChainCountFunction::regex() const { // {{{
	return "^!(c+)ount\\s+(.+)";
} // }}}


