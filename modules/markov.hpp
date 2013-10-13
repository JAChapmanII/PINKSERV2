#ifndef MODULES_MARKOV_HPP
#define MODULES_MARKOV_HPP

#include <iostream>
#include <vector>
#include "variable.hpp"
#include "markovmodel.hpp"

// #m: markov: markov data: markovLoad: markovSave
void markovLoad(std::istream &in);
void markovSave(std::ostream &out);

// print the markov model in plain text
std::ostream &dumpMarkov(std::ostream &out);
// load the markov model in plain text (append to current model)
std::istream &readMarkov(std::istream &in);

// #f: observe: observes a line and adds it into the markov model
Variable observe(std::vector<Variable> arguments);

// #f: markov: returns a markov chain given a seed string
Variable markov(std::vector<Variable> arguments);

// #f: respond: responds with a markov chain given a seed string
Variable respond(std::vector<Variable> arguments);

// #f: ccount: return number of markov chains
Variable ccount(std::vector<Variable> arguments);

// #f: correct: magically corrects you
Variable correct(std::vector<Variable> arguments);

// #f: dsize: return number of unique 1-grams
Variable dsize(std::vector<Variable> arguments);

// #f: rword: returns a random word (can be restricted to frequency range)
Variable rword(std::vector<Variable> arguments);

#endif // MODULES_MARKOV_HPP
