#ifndef MARKOVMODEL_HPP
#define MARKOVMODEL_HPP

#include <list>
#include <map>
#include <iostream>

/* MarkovModel:
 * 	handles markov model related functionality
 * 	holds all needed data to generate corpus-like chains
 *
 * terminology:
 * 	seed: the context to generate from
 * 	order: the length of the seed
 * 	endpoint: the result of generation
 * 	chain: the context plus the endpoint
 */
// TODO: template key/value?
class MarkovModel {
	public:
		MarkovModel();
		~MarkovModel();

		// TODO: sane default other than 1?
		// add some number of copies of chain
		void increment(std::list<unsigned> chain, unsigned amount = 1);

		// return count of chain
		unsigned count(std::list<unsigned> chain);

		// pick a random endpoint given a seed
		unsigned random(std::list<unsigned> seed);

		// calculate the probability of a chain occuring
		double probability(std::list<unsigned> chain);

		// return a smooth model for a seed
		std::map<unsigned, double> smooth(std::list<unsigned> seed);

		//bool contains(std::list<std::string> chain);

		// returns a partially resloved seed
		MarkovModel *operator[](std::list<unsigned> seed);
		MarkovModel *operator[](unsigned key);

		//typename std::map<unsigned, MarkovModel<O - 1>>::iterator begin();
		//typename std::map<unsigned, MarkovModel<O - 1>>::iterator end();

		// returns the number of endpoints at this level
		unsigned size() const;

		// save model to a file
		std::ostream &write(std::ostream &out);
		// read model from a file
		std::istream &read(std::istream &in);

	protected:
		// ensure a submodel exists
		void ensure(unsigned key);

		unsigned m_count; // occurrences of this endpoint
		std::map<unsigned, MarkovModel *> m_model; // further endpoints
};

#endif // MARKOVMODEL_HPP
