#ifndef MODULES_MARKOV_HPP
#define MODULES_MARKOV_HPP

#include <vector>
#include "variable.hpp"
#include "bot.hpp"

// #m: markov: markov functions

// #f: ngobserve: observes a line and adds it into the ngram model
void ngobserve(Bot *bot, std::string text);

// #f: ngrandom: return a random word given a prefix
std::string ngrandom(Bot *bot, std::string text);

// #f: ngmarkov: return a random string given a seed
std::string ngmarkov(Bot *bot, std::string text);


// #f: chainCount: count the number of times a chain occurs
Variable chainCount(std::vector<Variable> arguments);

// #f: prefixOptions: count the number of endpoints of a prefix
Variable prefixOptions(std::vector<Variable> arguments);

// #f: totalChains: return the total number of chains of all orders
Variable totalChains(std::vector<Variable> arguments);



// #f: respond: responds with a markov chain given a seed string
Variable respond(std::vector<Variable> arguments);

// #f: correct: magically corrects you
Variable correct(std::vector<Variable> arguments);

#endif // MODULES_MARKOV_HPP
