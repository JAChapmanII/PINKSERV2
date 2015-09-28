#ifndef MODULES_MARKOV_HPP
#define MODULES_MARKOV_HPP

#include <string>
#include "bot.hpp"

// #m: markov: markov functions

// #f: observe: observes a line and adds it into the ngram model
void observe(Bot *bot, std::string text);

// #f: unlearn: decrease the probability of a chain in the ngram model
void unlearn(Bot *bot, std::string text);

// #f: ngrandom: return a random word given a prefix
std::string ngrandom(Bot *bot, std::string text);

// #f: markov: return a random string given a seed
std::string markov(Bot *bot, std::string text);


// #f: chainCount: count the number of times a chain occurs
sqlite_int64 chainCount(Bot *bot, std::string chain_s);

// #f: prefixOptions: count the number of endpoints of a prefix
sqlite_int64 prefixOptions(Bot *bot, std::string prefix_s);

// #f: totalChains: return the total number of chains of all orders
sqlite_int64 totalChains(Bot *bot);



// #f: respond: responds with a markov chain given a seed string
std::string respond(Bot *bot, std::string text);

// #f: correct: magically corrects you
std::string correct(Bot *bot, std::string text);

#endif // MODULES_MARKOV_HPP
