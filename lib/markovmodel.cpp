#include "markovmodel.hpp"
using std::list;
using std::map;
using std::istream;
using std::ostream;

#include <random>
using std::uniform_real_distribution;
using std::uniform_int_distribution;

#include <string>
using std::string;

#include <limits>
using std::numeric_limits;

#include <algorithm>
using std::min;

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

#include <iostream>
using std::cerr;
using std::endl;
unsigned MarkovModel::random(list<unsigned> seed) {
	// obtain the smooth, normalized model
	map<unsigned, uint64_t> smoothModel = this->smooth(seed); // TODO
	if(smoothModel.empty()) {
		cerr << "returned smooth model was empty" << endl;
		return 0;
	}
	// find total and square model
	uint64_t total = 0, lt = 0;
	for(auto &i : smoothModel) {
		i.second *= i.second;
		total += i.second;
		if(total < lt)
			cerr << "oh god why" << endl;
		lt = total;
	}
	if(total < 1) { // TODO:
		cerr << "total is less than one" << endl;
		return 0;
	}
	// generate a random number between 0 and 1
	uniform_int_distribution<uint64_t> n(1, total);
	uint64_t r = n(global::rengine), oR = r;
	// lookup the key
	auto it = smoothModel.begin();
	map<unsigned, uint64_t>::iterator m = smoothModel.begin();
	uint64_t sT = 0;
	while(it != smoothModel.end() && it->second < r) {
		if(it->second > m->second)
			m = it;
		sT += it->second;
		r -= it->second, it++;
	}
	if(it == smoothModel.end()) {
		cerr << "total: " << total << endl;
		cerr << "run off end, using max: " << m->second << "-> " << m->first << endl;
		cerr << "r: " << oR << " -> " << r << endl;
		cerr << "smSize: " << smoothModel.size() << endl;
		cerr << "sT: " << sT << endl;
		return m->first;
		//throw (string)"fell off end";
	}
	// return the found key
	return it->first;
}

double MarkovModel::probability(list<unsigned> chain) {
	// remove the endpoint
	unsigned back = chain.back(); chain.pop_back();
	// get smooth model for endpoint, lookup probability
	return this->smooth(chain)[back];
}

// TODO: debug printing, max markov order....
// TODO: squaring large numbers can get very large
static unsigned maxMarkovOrder = 3;
map<unsigned, uint64_t> MarkovModel::smooth(list<unsigned> seed) {
	map<unsigned, uint64_t> smoothModel;
	// TODO: this is not how it should actually work.... PPM
	cerr << "start size: " << seed.size() << endl;
	while(seed.size() > maxMarkovOrder)
		seed.pop_front();
	cerr << "abbreviated size: " << seed.size() << endl;
	while(!seed.empty()) {
		uint64_t multiplier = 1;//pow(2, pow(2, seed.size() + 1)); TODO
		MarkovModel *submodel = (*this)[seed]; seed.pop_front();
		// couldn't find seed
		if(submodel == NULL)
			continue;;
		for(auto it : submodel->m_model) {
			if(submodel->m_model[it.first]->m_count < 1)
				continue;
			uint64_t v = submodel->m_model[it.first]->m_count * multiplier;
			smoothModel[it.first] += v;
		}
		if(smoothModel.empty())
			continue;
		cerr << "final size: " << seed.size() << endl;
		return smoothModel; // TOOD: highest order only for testing
	}
	cerr << "after main" << endl;
	if(smoothModel.empty()) {
		cerr << "adding 0 order" << endl;
		// 0 order model
		MarkovModel *submodel = (*this)[seed];
		// couldn't find seed
		if(submodel != NULL)
			for(auto it : submodel->m_model) {
				uint64_t v = submodel->m_model[it.first]->m_count;
				smoothModel[it.first] += v;
			}
	}

	//return normalize(smoothModel);
	return smoothModel;
}

map<unsigned, double> MarkovModel::rough(list<unsigned> seed) {
	int initialSize = seed.size();
	for(int i = 0; i <= initialSize; ++i) {
		MarkovModel *submodel = (*this)[seed];
		if(!seed.empty())
			seed.pop_front();
		// seed not found
		if(submodel == NULL)
			continue;
		if(submodel->m_model.size() == 0)
			continue;

		// translate model into result type
		map<unsigned, double> roughModel;
		for(auto it : submodel->m_model)
			roughModel[it.first] = it.second->m_count;

		return normalize(roughModel);
	}

	return map<unsigned, double>();
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
	unsigned modelSize = 0;
	brain::read(in, modelSize);
	for(unsigned i = 0; i < modelSize; ++i) {
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

map<unsigned, double> normalize(map<unsigned, double> model) {
	double gMin = numeric_limits<double>::max();
	for(auto i : model)
		gMin = min(gMin, i.second);
	cerr << "gMin: " << gMin << endl;
	double total = 0.0;
	for(auto i : model) {
		i.second /= gMin;
		i.second *= i.second;
		total += i.second;
	}

	map<unsigned, double> normal;
	for(auto i : model)
		normal[i.first] = i.second / total;
	return normal;
}

