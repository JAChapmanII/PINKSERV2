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

#include "config.hpp"
#include "util.hpp"
using util::split;
using util::join;
#include "brain.hpp"
#include "markovmodel.hpp"

// ( (tagged word)* | (start of string) ) (untagged word) -> (tag)
static double smoothingCoefficients[] = { 1.0, 1.5, 2.5 };
static const unsigned postModelOrder = 3;
static MarkovModel<postModelOrder> postModel;

void train(string taggedString);
string tag(queue<string> taggedWords, string word, bool atStart = false);
string tag(string untaggedSentence);

POSTFunction::POSTFunction() : Function(true) { // {{{
} // }}}
string POSTFunction::run(ChatLine line, smatch matches) { // {{{
	return line.nick + ": " + tag(matches[1]);
} // }}}
string POSTFunction::name() const { // {{{
	return "post";
} // }}}
string POSTFunction::help() const { // {{{
	return "Returns a part-of-speech tagged string";
} // }}}
string POSTFunction::regex() const { // {{{
	return "^!post\\s+(.+)$";
} // }}}
ostream &POSTFunction::output(ostream &out) { // {{{
	return postModel.write(out);
} // }}}
istream &POSTFunction::input(istream &in) { // {{{
	return postModel.read(in);
} // }}}

string POSTTrainFunction::run(ChatLine line, smatch matches) { // {{{
	train(matches[1]);
	return "";
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

void train(string taggedString) {
}
string tag(queue<string> taggedWords, string word, bool atStart) {
}
string tag(string untaggedSentence) {
	vector<string> words = split(untaggedSentence);
	if(words.empty())
		return "(error: untaggedSentence contains no words?)";

	queue<string> taggedWords;
	taggedWords.push(tag(taggedWords, words.front(), true));
	for(unsigned i = 1; i < words.size(); ++i)
		taggedWords.push(tag(taggedWords, words[i]));

	return join(taggedWords, " ");
}

