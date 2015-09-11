#include "modules.hpp"
using std::map;
using std::vector;
using std::string;

using modules::Module;

#include <iostream>
using std::cerr;
using std::endl;

#include "global.hpp"

map<string, InjectedFunction> modules::hfmap;
vector<Module> modules::modules;
static bool modules_inited = false;

Module findModule(string mname);

void defineModules();
void setupFunctions();

namespace modules {
	namespace IFHelper {
		template<> std::string coerce(std::vector<Variable> &vars) {
			//if(vars.empty())
				//throw std::string{"coerce: wanted string but has nothing"};
			std::string res = util::join(vars, " ");
			vars.clear();
			return res;
		}
		template<> long coerce(std::vector<Variable> &vars) {
			if(vars.empty())
				throw std::string{"coerce: wanted int but has nothing"};
			long var = vars.front().asInteger().value.l;
			vars.erase(vars.begin());
			return var;
		}
		template<> double coerce(std::vector<Variable> &vars) {
			if(vars.empty())
				throw std::string{"coerce: wanted int but has nothing"};
			double var = vars.front().asDouble().value.d;
			vars.erase(vars.begin());
			return var;
		}
	}
}

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

	//cerr << "    dictionary size: " << global::dictionary.size() << endl;

	setupFunctions();
	return true;
}

#include "modules.cpp.gen"

