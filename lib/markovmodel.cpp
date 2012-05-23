#include "markovmodel.hpp"

#include <random>
using std::uniform_int_distribution;

#include "global.hpp"
#include "util.hpp"
#include "brain.hpp"

// TODO: needed because of .imp?

void MarkovModel<0>::increment( // {{{
		std::queue<std::string> chain, std::string target, unsigned count) {
	this->m_model[global::dictionary[target]] += count;
	this->m_total += count;
} // }}}
void MarkovModel<0>::increment(std::queue<unsigned> chain, unsigned target,
		unsigned count) {
	this->m_model[target] += count;
	this->m_total += count;
}

std::string MarkovModel<0>::random(std::queue<std::string> chain) { // {{{
	if(this->m_total == 0) {
		std::cerr << "this std::map is empty!?" << std::endl;
		return "";
	}
	uniform_int_distribution<> uid(1, this->m_total);
	unsigned target = uid(global::rengine);
	auto i = this->m_model.begin();
	for(; (i != this->m_model.end()) && (target > i->second); ++i)
		target -= i->second;
	if(i == this->m_model.end()) {
		std::cerr << "fell off the bandwagon" << std::endl;
		std::cerr << "total: " << this->m_total << std::endl;
		return "";
	}
	unsigned word = i->first;
	return global::dictionary[word];
} // }}}
bool MarkovModel<0>::contains(std::queue<std::string> chain) { // {{{
	if(chain.empty())
		return false;
	return util::contains(this->m_model, global::dictionary[chain.back()]);
} // }}}
unsigned MarkovModel<0>::operator[](std::queue<std::string> chain) { // {{{
	if(chain.empty())
		return 0;
	return this->m_model[global::dictionary[chain.back()]];
} // }}}
std::map<unsigned, unsigned>::iterator MarkovModel<0>::begin() { // {{{
	return this->m_model.begin();
} // }}}
std::map<unsigned, unsigned>::iterator MarkovModel<0>::end() { // {{{
	return this->m_model.end();
} // }}}
unsigned MarkovModel<0>::size() const { // {{{
	return this->m_model.size();
} // }}}
std::ostream &MarkovModel<0>::write(std::ostream &out) { // {{{
	return brain::write(out, this->m_model);
} // }}}
std::istream &MarkovModel<0>::read(std::istream &in) { // {{{
	brain::read(in, this->m_model);
	for(auto i : this->m_model)
		this->m_total += i.second;
	return in;
} // }}}
std::map<unsigned, unsigned> MarkovModel<0>::endpoint(std::queue<std::string> chain) { // {{{
	return this->m_model;
} // }}}
std::map<unsigned, unsigned> MarkovModel<0>::endpoint(std::queue<unsigned> chain) {
	return this->m_model;
}
unsigned MarkovModel<0>::total(std::queue<std::string> chain) { // {{{
	return this->m_total;
} // }}}


