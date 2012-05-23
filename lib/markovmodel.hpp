#ifndef MARKOVMODEL_HPP
#define MARKOVMODEL_HPP

#include <queue>
#include <string>
#include <map>
#include <iostream>

template<int O> class MarkovModel {
	public:
		MarkovModel() : m_order(O), m_model() { }
		void increment(std::queue<unsigned> chain, unsigned target,
				unsigned count = 1, bool atStart = false);
		void increment(std::queue<std::string> chain, std::string target,
				unsigned count = 1, bool atStart = false);
		std::string random(std::queue<std::string> chain);

		bool contains(std::queue<std::string> chain);
		unsigned operator[](std::queue<std::string> chain);
		MarkovModel<O - 1> operator[](std::string word);
		MarkovModel<O - 1> operator[](unsigned word);
		typename std::map<unsigned, MarkovModel<O - 1>>::iterator begin();
		typename std::map<unsigned, MarkovModel<O - 1>>::iterator end();
		unsigned size() const;

		std::ostream &write(std::ostream &out);
		std::istream &read(std::istream &in);

		std::map<unsigned, unsigned> endpoint(std::queue<std::string> chain);
		std::map<unsigned, unsigned> endpoint(std::queue<unsigned> chain);
		unsigned total(std::queue<std::string> chain);

	protected:
		unsigned m_order;
		std::map<unsigned, MarkovModel<O - 1>> m_model;
};

template<> class MarkovModel<0> {
	public:
		MarkovModel() : m_model(), m_total(0) { }

		void increment(std::queue<unsigned> chain, unsigned target,
				unsigned count = 1);
		void increment(std::queue<std::string> chain, std::string target,
				unsigned count = 1);
		std::string random(std::queue<std::string> chain);

		bool contains(std::queue<std::string> chain);
		unsigned operator[](std::queue<std::string> chain);
		std::map<unsigned, unsigned>::iterator begin();
		std::map<unsigned, unsigned>::iterator end();
		unsigned size() const;

		std::ostream &write(std::ostream &out);
		std::istream &read(std::istream &in);

		std::map<unsigned, unsigned> endpoint(std::queue<std::string> chain);
		std::map<unsigned, unsigned> endpoint(std::queue<unsigned> chain);
		unsigned total(std::queue<std::string> chain);

	protected:
		std::map<unsigned, unsigned> m_model;
		unsigned m_total;
};

#include "markovmodel.imp"

#endif // MARKOVMODEL_HPP
