#ifndef MODULES_MARKOV_HPP
#define MODULES_MARKOV_HPP

#include <iostream>
#include <vector>
#include <string>

// #mload: markovLoad
std::istream &markovLoad(std::istream &in);
// #msave: markovSave
std::ostream &markovSave(std::ostream &out);


// f#: markov: returns a markov chain given a seed string
std::string markov(std::vector<std::string> arguments);

// f#: ccount: return number of markov chains
std::string ccount(std::vector<std::string> arguments);

// #f: correct: magically corrects you
std::string correct(std::vector<std::string> arguments);

// #f: dsize: return number of unique 1-grams
std::string dsize(std::vector<std::string> arguments);

// #f: rword: returns a random word (can be restricted to frequency range)
std::string rword(std::vector<std::string> arguments);

#endif // MODULES_MARKOV_HPP
