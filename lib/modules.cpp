#include "modules.hpp"
using std::map;
using std::vector;
using std::string;
using std::istream;
using std::ostream;
using std::fstream;
using std::ifstream;
using std::ofstream;

using modules::Function;
using modules::Module;

#include <iostream>
using std::cerr;
using std::endl;

#include "config.hpp"
#include "global.hpp"

map<string, Function> modules::hfmap;
vector<Module> modules::modules;
static bool modules_inited = false;

Module findModule(string mname);
bool moduleLoaded(string mname);

void defineModules();
void setupFunctions();

void none(istream &brain);
void none(ostream &brain);
void none(istream &brain) { if(brain.bad()) return; }
void none(ostream &brain) { if(brain.bad()) return; }

Module findModule(std::string mname) {
	for(auto mod : modules::modules)
		if(mod.name == mname)
			return mod;
	throw (string)"module " + mname + " nonexistant";
}

bool moduleLoaded(std::string mname) {
	return findModule(mname).loaded;
}

bool modules::init(std::string brainFileName) {
	if(modules_inited)
		return true;

	cerr << "moudles::init: " << brainFileName << endl;
	defineModules();

	ifstream brain(brainFileName, fstream::binary);

	uint8_t hasDict = false;
	if(!brain.eof() && brain.good())
		hasDict = brain.get();
	if(!brain.eof() && brain.good() && hasDict)
		global::dictionary.read(brain);

	cerr << "\tread dictionary, size is: " << global::dictionary.size() << endl;
	cerr << "\treading modules:" << endl;
	unsigned read = 0;
	while(!brain.eof() && brain.good()) {
		// TODO: simplify this using brain:: for strings?
		// find length of next module name
		int length = brain.get();
		// if we hit the end of the file, we're done
		if(!brain.good())
			break;

		// read in the name of the module
		string mname;
		for(int i = 0; i < length; ++i) {
			int c = brain.get();
			// TODO: this isn't good
			if(!brain.good())
				break;
			mname += (string)"" + (char)c;
		}

		try {
			// TODO: error handling?
			Module mod = findModule(mname);
			cerr << "\t\tloading " << mod.name << " (" << mod.desc << ")" << endl;
			mod.load(brain);
			mod.loaded = true;
			read++;
		} catch(string &s) {
			cerr << "error: " << s << endl;
			throw s;
		}
	}
	cerr << "\t\t" << read << " modules read" << endl;
	
	cerr << endl;
	setupFunctions();
	return true;
}

bool modules::deinit(std::string brainFileName) {
	ofstream brain(brainFileName, fstream::binary | fstream::trunc);
	cerr << "modules::deinit: " << brainFileName << endl;

	// TODO: uhhh....
	brain.put('y');
	global::dictionary.write(brain);
	cerr << "\twrote dictionary, size is: " << global::dictionary.size() << endl;

	cerr << "\twriting modules: " << endl;
	unsigned wrote = 0;
	if(brain.good()) {
		for(auto mod : modules) {
			// TODO: simplify by using brain::?
			string name = mod.name;
			unsigned char length = name.length();
			brain << length;
			for(int i = 0; i < length; ++i)
				brain << name[i];
			cerr << "\t\twriting " << mod.name << " (" << mod.desc << ")" << endl;
			mod.save(brain);
			wrote++;
		}
	} else
		throw (string)"error: brain not good";
	cerr << "\t\t" << wrote << " modules wrote" << endl;

	return true;
}

#include "modules.cpp.gen"

