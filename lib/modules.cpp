#include "modules.hpp"
using std::string;

#include <iostream>
using std::cerr;
using std::endl;

#include <fstream>
using std::ifstream;
using std::ofstream;
using std::fstream;

#include "config.hpp"
#include "global.hpp"

//std::map<string, Function *> modules::map;
static bool modules_inited = false;

bool modules::init(std::string fileName) {
	if(modules_inited)
		return true;

	// TODO: generate function to call which initaliazes things?

	cerr << "  init: " << endl;
	/*
	ifstream in(fileName, fstream::binary);
	uint8_t hasDict = false;
	if(!in.eof() && in.good())
		hasDict = in.get();
	if(!in.eof() && in.good() && hasDict)
		global::dictionary.read(in);
	cerr << "    read dictionary" << endl;
	cerr << "    modules: " << endl;
	while(!in.eof() && in.good()) {
		int length = in.get();
		if(!in.good()) {
			break;
		}
		string name;
		for(int i = 0; i < length; ++i) {
			int c = in.get();
			if(!in.good()) {
				break;
			}
			name += (string)"" + (char)c;
		}
		Function *f = NULL;
		for(auto module : map)
			if(module.second->name() == name)
				f = module.second;
		if(f != NULL) {
			in >> *f;
		} else {
			cerr << " (unable to find function! " << name << ") " << endl;
			break;
		}
		cerr << " " << name;
	}
	*/
	cerr << endl;
	return true;
}

bool modules::deinit(std::string fileName) {
	/*
	ofstream out(fileName, fstream::binary | fstream::trunc);
	cerr << "  DEinit: " << endl;
	out.put('y');
	global::dictionary.write(out);
	cerr << "    wrote dictionary" << endl;
	cerr << "    modules: " << endl;
	if(out.good()) {
		for(auto m : map) {
			string name = m.second->name();
			unsigned char length = name.length();
			out << length;
			for(int i = 0; i < length; ++i)
				out << name[i];
			out << (*m.second);
			cerr << " " << name;
		}
	} else
		cerr << "out not good";
	cerr << endl;

	for(auto m : map)
		delete m.second;
	*/
	return true;
}

