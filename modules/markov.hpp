#ifndef MODULES_MARKOV_HPP
#define MODULES_MARKOV_HPP

#include <iostream>
#include <vector>
#include <string>

// #m: markov: markov data: markovLoad: markovSave
void markovLoad(std::istream &in);
void markovSave(std::ostream &out);

// observes a line and adds it into the markov model
void observe(std::string text);

// #f: markov: returns a markov chain given a seed string
std::string markov(std::vector<std::string> arguments);

// #f: ccount: return number of markov chains
std::string ccount(std::vector<std::string> arguments);

// #f: correct: magically corrects you
std::string correct(std::vector<std::string> arguments);

// #f: dsize: return number of unique 1-grams
std::string dsize(std::vector<std::string> arguments);

// #f: rword: returns a random word (can be restricted to frequency range)
std::string rword(std::vector<std::string> arguments);

#endif // MODULES_MARKOV_HPP
