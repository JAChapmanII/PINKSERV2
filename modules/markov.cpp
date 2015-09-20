#include "markov.hpp"
using std::vector;
using std::string;

#include <random>
using std::generate_canonical;

#include <iostream>
using std::cerr;
using std::endl;

#include "util.hpp"


// TODO: turn into pbrane variable
static int ngObserveMaxOrder = 4;
static unsigned long long totalIncrements = 0;
static string lastNGObserve = "";
static long long ng_timestamp = 0;

void learn(Bot *bot, vector<word_t> words, int increment = 1);
void learn(Bot *bot, vector<word_t> words, int increment) {
	{
		auto tran = bot->db.transaction();

		for(int i = 0; i < (int)words.size(); ++i) {
			vector<word_t> prefix;
			ngram_t ngram{prefix, words[i]};
			if(ngram.atom != (word_t)Anchor::Start
					&& ngram.atom != (word_t)Anchor::End)
				bot->ngStore.increment(ngram, increment);
			totalIncrements++;

			for(int j = i + 1; j < (int)words.size(); ++j) {
				ngram.prefix.push_back(ngram.atom);
				ngram.atom = words[j];
				if(ngram.order() > ngObserveMaxOrder)
					break;
				bot->ngStore.increment(ngram, increment);
				totalIncrements++;
			}
		}
	}

	if(bot->clock.now() > ng_timestamp + 10) {
		ng_timestamp = bot->clock.now();
		cerr << ng_timestamp << " stats: "
			<< bot->journal.size() << "L, "
			<< totalIncrements << "I" << endl;
	}
}

prefix_t make_prefix(Bot *bot, string text);
prefix_t make_prefix(Bot *bot, string text) {
	auto words = util::split(text);

	prefix_t prefix;
	for(auto &word : words)
		prefix.push_back(bot->dictionary[word]);
	return prefix;
}

string make_string(Bot *bot, prefix_t words);
string make_string(Bot *bot, prefix_t words) {
	// return the generated string
	string rs;
	for(auto &word : words)
		rs += bot->dictionary[word] + " ";
	return rs;
}

prefix_t generateSentence(Bot *bot, prefix_t words);
prefix_t generateSentence(Bot *bot, prefix_t words) {
	// TODO: newlines, respond
	auto result = words;

	while(true && result.size() < 32) {
		// check to see if we should try to pick another one or just be done
		double prob = 0.999;

		int rc = -1;
		while((rc = bot->ngStore.random(words, bot->rengine)) == -1) {
			if(words.empty())
				break;
			cerr << "stepping down from: " << words.size() << endl;
			words.erase(words.begin());
			prob *= .98;
		}
		if(rc == (int)Anchor::Start) continue;
		if(rc == (int)Anchor::End) {
			size_t len = result.size() - words.size();
			if(generate_canonical<double, 10>(bot->rengine) < 0.05 * (len + 1)) {
				cerr << "ngmarkov: end break" << endl;
				break;
			} else {
				cerr << "ngmarkov: failed end break: " << (len + 1) << endl;
				continue;
			}
		}

		result.push_back(rc);
		words.push_back(rc);

		//if(chain.size() <= maxMarkovOrder * 2.5)
			//prob += (1 - prob) / 2.0;
		if(result.size() >= 25)
			prob /= 10;

		string nexts = bot->dictionary[rc];
		// if we're currently ending with a punctuation, greatly increase the
		// chance of ending and sounding somewhat coherent
		if(((string)".?!;").find(nexts.back()) != string::npos)
			prob /= 1.85;

		// if a random num in [0, 1] is above our probability of ending, end
		if(generate_canonical<double, 16>(bot->rengine) > prob)
			break;
	}

	return result;
}


void observe(Bot *bot, string text) {
	auto words = make_prefix(bot, text);
	words.insert(words.begin(), (word_t)Anchor::Start);
	words.push_back((word_t)Anchor::End);

	learn(bot, words, 1);
}

void unlearn(Bot *bot, string text) {
	auto words = make_prefix(bot, text);

	learn(bot, words, -4);
}

string ngrandom(Bot *bot, string text) {
	auto prefix = make_prefix(bot, text);
	word_t res = bot->ngStore.random(prefix, bot->rengine);
	return bot->dictionary[res];
}

string markov(Bot *bot, string text) {
	auto words = make_prefix(bot, text);
	auto sentence = generateSentence(bot, words);
	return make_string(bot, sentence);
}

string respond(Bot *bot, string text) {
	auto words = make_prefix(bot, text);
	words.push_back((word_t)Anchor::End);
	words.push_back((word_t)Anchor::Start);

	auto sentence = generateSentence(bot, words);
	sentence.erase(sentence.begin(), sentence.begin() + words.size());

	return make_string(bot, sentence);
}


sqlite_int64 chainCount(Bot *bot, string chain_s) {
	auto words_s = util::split(chain_s);
	if(words_s.size() < 1)
		throw string{"chainCount requires a chain (at least one word)}"};

	word_t atom = bot->dictionary[words_s.back()];
	words_s.pop_back();

	prefix_t prefix;
	prefix.reserve(words_s.size());
	for(auto &word : words_s)
		prefix.push_back(bot->dictionary[word]);

	ngram_t ngram{prefix, atom};

	chain_t chain = bot->ngStore.fetch(ngram);
	return chain.count;
}
sqlite_int64 prefixOptions(Bot *bot, string prefix_s) {
	throw (string)"error: prefixOptions unimplemented"; // TODO implement
}
sqlite_int64 totalChains(Bot *bot) {
	throw (string)"error: totalChains unimplemented"; // TODO implement
}



string correct(Bot *bot, string text) {
	throw (string)"error: correct unimplemented"; // TODO: implement
}

