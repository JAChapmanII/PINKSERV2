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

#include <arpa/inet.h>

#include "util.hpp"
using util::split;

ostream &ssummOut(ostream &out, map<string, map<string, unsigned>> ssumm);
istream &ssummIn(istream &in, map<string, map<string, unsigned>> &ssumm);

ostream &sumOut(ostream &out, map<string, unsigned> sum);
istream &sumIn(istream &in, map<string, unsigned> &sum);

ostream &sOut(ostream &out, string s);
istream &sIn(istream &in, string &s);

ostream &uOut(ostream &out, unsigned u);
istream &uIn(istream &in, unsigned &u);

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
void MarkovFunction::passive(global::ChatLine line, bool parsed) {
	if(!parsed)
		insert(line.text);
}
string MarkovFunction::name() const { // {{{
	return "markov";
} // }}}
string MarkovFunction::help() const { // {{{
	return "Returns a markov chain.";
} // }}}
string MarkovFunction::regex() const { // {{{
	return "^!markov\\s+(.*)";
} // }}}
ostream &MarkovFunction::output(ostream &out) {
	ssummOut(out, markovModel);
	return out;
}
istream &MarkovFunction::input(istream &in) {
	ssummIn(in, markovModel);
	return in;
}


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


ostream &ssummOut(ostream &out, map<string, map<string, unsigned>> ssumm) { // {{{
	uOut(out, ssumm.size());
	for(auto i : ssumm) {
		sOut(out, i.first);
		sumOut(out, i.second);
	}
	return out;
} // }}}
istream &ssummIn(istream &in, map<string, map<string, unsigned>> &ssumm) { // {{{
	unsigned size = 0;
	uIn(in, size);
	for(unsigned i = 0; i < size; ++i) {
		string initial;
		map<string, unsigned> sum;

		sIn(in, initial);
		sumIn(in, sum);
		ssumm[initial] = sum;
	}
	return in;
} // }}}

ostream &sumOut(ostream &out, map<string, unsigned> sum) { // {{{
	uOut(out, sum.size());
	for(auto i : sum) {
		sOut(out, i.first);
		uOut(out, i.second);
	}
	return out;
} // }}}
istream &sumIn(istream &in, map<string, unsigned> &sum) { // {{{
	unsigned size = 0;
	uIn(in, size);
	for(unsigned i = 0; i < size; ++i) {
		string end;
		unsigned count;

		sIn(in, end);
		uIn(in, count);
		sum[end] = count;
	}
	return in;
} // }}}

ostream &sOut(ostream &out, string s) { // {{{
	unsigned char length = s.length();
	out << length;
	for(int i = 0; i < length; ++i)
		out << s[i];
	return out;
} // }}}
istream &sIn(istream &in, string &s) { // {{{
	unsigned char length = in.get();
	s = "";
	for(int i = 0; i < length; ++i) {
		int c = in.get();
		if(c != -1)
			s += (char)c;
	}
	return in;
} // }}}

ostream &uOut(ostream &out, unsigned u) { // {{{
	uint32_t no = htonl(u);
	unsigned char *noc = (unsigned char *)&no;
	for(int i = 0; i < 4; ++i)
		out << noc[i];
	return out;
} // }}}
istream &uIn(istream &in, unsigned &u) { // {{{
	unsigned char noc[4];
	for(int i = 0; i < 4; ++i)
		noc[i] = in.get();
	uint32_t no = *(uint32_t *)noc;
	u = ntohl(no);
	return in;
} // }}}

