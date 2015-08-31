#include "modules.hpp"
using std::map;
using std::vector;
using std::string;

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

void defineModules();
void setupFunctions();

Module findModule(std::string mname) {
	for(auto mod : modules::modules)
		if(mod.name == mname)
			return mod;
	throw (string)"module " + mname + " nonexistant";
}

bool modules::init() {
	if(modules_inited)
		return true;

	cerr << "moudles::init: " << endl;
	defineModules();

	//cerr << "\tread dictionary, size is: " << global::dictionary.size() << endl; TODO
	//cerr << "\treading modules:" << endl; TODO: generic module init/deinit?

	setupFunctions();
	return true;
}

bool modules::deinit() {
	cerr << "modules::deinit: " << endl;

	// TODO: uhhh....
	//cerr << "\twrote dictionary, size is: " << global::dictionary.size() << endl;
	// TODO: module de-init?

	return true;
}

#include "modules.cpp.gen"

