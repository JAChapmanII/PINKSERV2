#include "markovmodel.hpp"
using std::list;
using std::map;
using std::istream;
using std::ostream;

#include <random>
using std::uniform_real_distribution;

#include <string>
using std::string;

#include "global.hpp"
#include "util.hpp"
#include "brain.hpp"

		unsigned m_count; // occurrences of this endpoint
		std::map<unsigned, MarkovModel *> m_model; // further endpoints

MarkovModel::MarkovModel() : m_count(0), m_model() {
}
MarkovModel::~MarkovModel() {
	for(auto it : this->m_model)
		delete it.second;
}

void MarkovModel::increment(list<unsigned> chain, unsigned amount) {
	// nothing to do
	if(chain.empty())
		return;

	unsigned f = chain.front(); chain.pop_front();
	// make sure the submodel exists
	this->ensure(f);

	// we're at the end
	if(chain.empty()) {
		this->m_model[f]->m_count += amount;
		return;
	}
	// increment the subchain on the submodel
	this->m_model[f]->increment(chain, amount);
}

unsigned MarkovModel::count(list<unsigned> chain) {
	// we've arrived
	if(chain.empty())
		return this->m_count;
	unsigned f = chain.front(); chain.pop_front();
	if(this->m_model.find(f) == this->m_model.end())
		return 0; // doesn't exist
	// give count from submodel
	return this->m_model[f]->count(chain);
}

unsigned MarkovModel::random(list<unsigned> seed) {
	// obtain the smooth, normalized model
	map<unsigned, double> smoothModel = this->smooth(seed);
	if(smoothModel.empty())
		return 0;
	// generate a random number between 0 and 1
	uniform_real_distribution<double> n(0, 1);
	double r = n(global::rengine);
	// lookup the key
	auto it = smoothModel.begin();
	while(it != smoothModel.end() && it->second < r) {
		r -= it->second, it++;
	}
	if(it == smoothModel.end())
		throw (string)"fell off end";
	// return the found key
	return it->first;
}

double MarkovModel::probability(list<unsigned> chain) {
	// remove the endpoint
	unsigned end = chain.back(); chain.pop_back();
	// get smooth model for endpoint, lookup probability
	return this->smooth(chain)[end];
}

map<unsigned, double> MarkovModel::smooth(list<unsigned> seed) {
	map<unsigned, double> smooth;
	// TODO: this is not how it should actually work.... PPM
	double multiplier = 16.0, total = 0;
	while(!seed.empty()) {
		MarkovModel *end = (*this)[seed]; seed.pop_front();
		// couldn't find seed
		if(end == NULL)
			continue;;
		for(auto it : end->m_model) {
			double v = end->m_model[it.first]->m_count * multiplier;
			total += v;
			smooth[it.first] += v;
		}
		multiplier /= 12.0;
	}
	if(smooth.size() == 0)
		return smooth;

	// normalize values so they sum to 1
	map<unsigned, double> normalized;
	for(auto it : smooth)
		normalized[it.first] = it.second / total;
	return normalized;
}

MarkovModel *MarkovModel::operator[](list<unsigned> seed) {
	// we have no seed, return us
	if(seed.empty())
		return this;
	// get front, remove from seed
	unsigned f = seed.front(); seed.pop_front();
	// can't find seed
	if(this->m_model.find(f) == this->m_model.end())
		return NULL;
	// get seed out of sub-model
	return (*this->m_model[f])[seed];
}
MarkovModel *MarkovModel::operator[](unsigned key) {
	// build a 1-element list and use normal operator[]
	list<unsigned> seed;
	seed.push_back(key);
	return (*this)[seed];
}

map<unsigned, MarkovModel *>::iterator MarkovModel::begin() {
	return this->m_model.begin();
}
map<unsigned, MarkovModel *>::iterator MarkovModel::end() {
	return this->m_model.end();
}

unsigned MarkovModel::size() const {
	return this->m_model.size();
}

ostream &MarkovModel::write(ostream &out) {
	brain::write(out, this->m_count);
	brain::write(out, this->size());
	for(auto it : this->m_model) {
		brain::write(out, it.first);
		it.second->write(out);
	}
	return out;
}
istream &MarkovModel::read(istream &in) {
	brain::read(in, this->m_count);
	unsigned size = 0;
	brain::read(in, size);
	for(unsigned i = 0; i < size; ++i) {
		unsigned key = 0;
		brain::read(in, key);
		this->ensure(key);
		this->m_model[key]->read(in);
	}
	return in;
}

void MarkovModel::ensure(unsigned key) {
	if(this->m_model.find(key) == this->m_model.end())
		this->m_model[key] = new MarkovModel();
}

