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
static long long ng_timestamp = 0;
static string lastNGObserve = "";


void observe(Bot *bot, string text) {
	auto toObserve = text;

	vector<string> words_s = util::split(toObserve);
	vector<word_t> words; words.reserve(words_s.size());
	words.push_back((word_t)Anchor::Start);
	for(auto &word : words_s) words.push_back(bot->dictionary[word]);
	words.push_back((word_t)Anchor::End);

	{
		auto tran = bot->db.transaction();

		for(int i = 0; i < (int)words.size(); ++i) {
			vector<word_t> prefix;
			ngram_t ngram{prefix, words[i]};
			if(ngram.atom != (word_t)Anchor::Start
					&& ngram.atom != (word_t)Anchor::End)
				bot->ngStore.increment(ngram);
			totalIncrements++;

			for(int j = i + 1; j < (int)words.size(); ++j) {
				ngram.prefix.push_back(ngram.atom);
				ngram.atom = words[j];
				if(ngram.order() > ngObserveMaxOrder)
					break;
				bot->ngStore.increment(ngram);
				totalIncrements++;
			}
		}
	}

	if(bot->clock.now() > ng_timestamp + 10) {
		ng_timestamp = bot->clock.now();
		cerr << ng_timestamp << " checkpointing... ("
			<< bot->journal.size() << "L, " << totalIncrements << "I)" << endl;
		sqlite3_wal_checkpoint(bot->db.getDB(), nullptr);
	}

	//lastNGObserve = now;
}

string ngrandom(Bot *bot, string text) {
	auto arguments = util::split(text);
	prefix_t words; words.reserve(arguments.size());
	for(auto &word : arguments)
		words.push_back(bot->dictionary[word]);

	word_t res = bot->ngStore.random(words, bot->rengine);
	return bot->dictionary[res];
}
string markov(Bot *bot, string text) {
	// TODO: newlines, respond
	auto arguments = util::split(text);

	prefix_t words, result;
	words.reserve(arguments.size());
	result.reserve(arguments.size() * 3);
	for(auto &word : arguments) {
		word_t w = bot->dictionary[word];
		words.push_back(w);
		result.push_back(w);
	}

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
		if(rc == (int)Anchor::End) {
			cerr << "ngmarkov: end break" << endl;
			break;
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

	// return the generated string
	string rs;
	for(auto &word : result)
		rs += bot->dictionary[word] + " ";
	return rs;
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



string respond(Bot *bot, string text) {
	throw (string)"error: respond unimplemented"; // TODO: implement
}
string correct(Bot *bot, string text) {
	throw (string)"error: correct unimplemented"; // TODO: implement
}

