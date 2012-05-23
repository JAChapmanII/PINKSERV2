#include "post.hpp"
using std::string;
using std::ostream;
using std::istream;
using boost::smatch;

#include <random>
using std::uniform_real_distribution;
using std::uniform_int_distribution;
using std::generate_canonical;

#include <queue>
using std::queue;

#include <vector>
using std::vector;

#include <map>
using std::map;

#include <utility>
using std::make_pair;
using std::pair;

#include "config.hpp"
#include "util.hpp"
using util::split;
using util::join;
using util::last;
#include "brain.hpp"
#include "markovmodel.hpp"
#include "global.hpp"
using global::dictionary;

// ( (tagged word)* | (start of string) ) (untagged word) -> (tag)
static unsigned  tagSmoothingCoefficients[] = { 1, 2,  5, 10 };
static unsigned wtagSmoothingCoefficients[] = { 3, 6, 15, 30 };
static const unsigned postModelOrder = 3;
static MarkovModel<postModelOrder> postModel;

string getWord(string taggedWord);
string getTag(string taggedWord);

static void push(vector<pair<unsigned, unsigned>> taggedWords, unsigned order);
static void train(string taggedString);
static unsigned merge(map<unsigned, unsigned> &into,
		map<unsigned, unsigned> from, unsigned coefficient);
static unsigned merge(map<unsigned, unsigned> &into,
		MarkovModel<0> from, unsigned coefficient);
static map<unsigned, unsigned> tagOnly(
		vector<pair<unsigned, unsigned>> taggedWords);
static unsigned tag(vector<pair<unsigned, unsigned>> taggedWords, unsigned word);
static string   tag(string untaggedSentence);

POSTFunction::POSTFunction() : Function(true) { // {{{
} // }}}
string POSTFunction::run(ChatLine line, smatch matches) { // {{{
	string text = matches[2];
	if(text.empty())
		text = global::lastLog.back().text;
	return line.nick + ": " + tag(text);
} // }}}
string POSTFunction::name() const { // {{{
	return "post";
} // }}}
string POSTFunction::help() const { // {{{
	return "Returns a part-of-speech tagged string";
} // }}}
string POSTFunction::regex() const { // {{{
	return "^!post(\\s+(.+))?$";
} // }}}
ostream &POSTFunction::output(ostream &out) { // {{{
	return postModel.write(out);
} // }}}
istream &POSTFunction::input(istream &in) { // {{{
	return postModel.read(in);
} // }}}

string POSTTrainFunction::run(ChatLine line, smatch matches) { // {{{
	train(matches[1]);
	return line.nick + ": thank you for your input";
} // }}}
string POSTTrainFunction::name() const { // {{{
	return "post_train";
} // }}}
string POSTTrainFunction::help() const { // {{{
	return "Add a training sentence to the POST model";
} // }}}
string POSTTrainFunction::regex() const { // {{{
	return "^!post_train\\s+(.+)$";
} // }}}


string getWord(string taggedWord) { // {{{
	vector<string> tokens = split(taggedWord, "/");
	// remove last token (the actual tag)
	tokens.pop_back();
	// join the word-pieces back together
	return join(tokens, "/");
} // }}}
string getTag(string taggedWord) { // {{{
	vector<string> tokens = split(taggedWord, "/");
	// return the last token (the tag)
	return tokens.back();
} // }}}

void push(vector<pair<unsigned, unsigned>> taggedWords, unsigned order) { // {{{
	if(taggedWords.size() < order)
		return;

	// special case, order == 0. Simply insert { empty -> tag } for all in tw
	if(order == 0) {
		queue<unsigned> empty;
		for(auto i : taggedWords)
			postModel.increment(empty, i.second);
		return;
	}

	// i is the start point in the taggedWords sequence
	for(unsigned i = 0; i < taggedWords.size() - order; ++i) {
		queue<unsigned> chain;
		// initial run of tags
		for(unsigned j = 0; j < order - 1; ++j)
			chain.push(taggedWords[i].second);
		// current word under scrutiny
		chain.push(taggedWords[order - 1].first);
		postModel.increment(chain, taggedWords[order].second);
	}

	// "start" chain
	queue<unsigned> chain;
	chain.push(Dictionary<string, unsigned>::Start);
	for(unsigned i = 0; i < order - 1; ++i)
		chain.push(taggedWords[i].second);
	chain.push(taggedWords[order - 1].first);
	postModel.increment(chain, taggedWords[order].second);
} // }}}

void train(string taggedString) { // {{{
	vector<string> words = split(taggedString);
	if(words.empty())
		return;

	vector<pair<unsigned, unsigned>> taggedWords;
	for(auto i : words)
		taggedWords.push_back(make_pair(
					dictionary[getWord(i)], dictionary[getTag(i)]));

	for(unsigned o = 0; o < postModelOrder; ++o)
		push(taggedWords, o);
} // }}}

using std::cerr;
using std::endl;
void printQString(queue<unsigned> uqueue, bool pendl = true) { // {{{
	while(!uqueue.empty()) {
		cerr << dictionary[uqueue.front()] << ", ";
		uqueue.pop();
	}
	if(pendl)
		cerr << endl;
} // }}}
queue<unsigned> prependStart(queue<unsigned> uqueue) { // {{{
	queue<unsigned> ret;
	ret.push(Dictionary<string, unsigned>::Start);
	while(uqueue.size() > 1) {
		ret.push(uqueue.front());
		uqueue.pop();
	}
	return ret;
} // }}}

unsigned merge(map<unsigned, unsigned> &into, map<unsigned, unsigned> from, // {{{
		unsigned coefficient) {
	unsigned added = 0;
	for(auto i : from) {
		into[i.first] += i.second * coefficient;
		added         += i.second * coefficient;
	}
	return added;
} // }}}
unsigned merge(map<unsigned, unsigned> &into, MarkovModel<0> from, // {{{
		unsigned coefficient) {
	unsigned added = 0;
	for(auto i : from) {
		into[i.first] += i.second * coefficient;
		added         += i.second * coefficient;
	}
	return added;
} // }}}

unsigned tag(vector<pair<unsigned, unsigned>> taggedWords, unsigned word) { // {{{
	map<unsigned, unsigned> smoothModel;
	unsigned smoothModelTotal = 0;
	unsigned target = word;

	if(taggedWords.size() > postModelOrder)
		taggedWords = last(taggedWords, postModelOrder);

	if(taggedWords.size() >= 2) {
		vector<pair<unsigned, unsigned>> t = last(taggedWords, 2);
		queue<unsigned> l;
		for(auto i : t)
			l.push(i.second);
		MarkovModel<2> a = postModel[l.front()]; l.pop();
		MarkovModel<1> b =         a[l.front()]; l.pop();
		for(auto c : b)
			smoothModelTotal += merge(smoothModel, c.second,
					tagSmoothingCoefficients[2]);
	}
	if(taggedWords.size() >= 1) {
		vector<pair<unsigned, unsigned>> t = last(taggedWords, 1);
		queue<unsigned> l;
		for(auto i : t)
			l.push(i.second);
		MarkovModel<2> a = postModel[dictionary[""]];
		MarkovModel<1> b =         a[l.front()];
		for(auto c : b)
			smoothModelTotal += merge(smoothModel, c.second,
					tagSmoothingCoefficients[1]);
	}
	if(taggedWords.size() >= 0) {
		MarkovModel<2> a = postModel[dictionary[""]];
		MarkovModel<1> b =         a[dictionary[""]];
		for(auto c : b)
			smoothModelTotal += merge(smoothModel, c.second,
					tagSmoothingCoefficients[0]);
	}


	for(unsigned o = 0; o < postModelOrder - 1; ++o) {
		vector<pair<unsigned, unsigned>> relevant = last(taggedWords, o);
		queue<unsigned> chain;
		for(auto i : relevant)
			chain.push(i.second);
		chain.push(target);

		smoothModelTotal += merge(smoothModel, postModel.endpoint(chain),
				wtagSmoothingCoefficients[o]);
	}

	// if we have nothing about that seed, return nothing
	if(smoothModel.empty()) {
		cerr << "smoothModel for taggedWords -> " << word << " is empty" << endl;
		return dictionary[""];
	}

	// pick a random number in [0, total)
	// TODO: unsigned now
	uniform_real_distribution<> urd(0, smoothModelTotal);
	double r = urd(global::rengine);

	// find the end point corresponding to that
	auto i = smoothModel.begin();
	for(; (i != smoothModel.end()) && (r >= i->second); ++i)
		r -= i->second;
	if(i == smoothModel.end()) {
		global::err << "post::tag: oh shit ran off the end of ends!" << std::endl;
		return dictionary[""];
	}
	return i->first;
} // }}}
string tag(string untaggedSentence) { // {{{
	vector<string> words = split(untaggedSentence);
	if(words.empty())
		return "(error: untaggedSentence contains no words?)";

	vector<pair<unsigned, unsigned>> taggedWords;
	taggedWords.push_back(make_pair(0, Dictionary<string, unsigned>::Start));
	for(auto word : words)
		taggedWords.push_back(make_pair(
					dictionary[word], tag(taggedWords, dictionary[word])));

	string result = "";
	for(unsigned i = 1; i < taggedWords.size(); ++i)
		result += dictionary[taggedWords[i].first] + "/" +
			dictionary[taggedWords[i].second] + " ";

	return result;
} // }}}

