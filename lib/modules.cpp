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
void setupFunctions(Bot *bot);

namespace modules {
	namespace IFHelper {
		template<> string coerce(vector<Variable> &vars) {
			//if(vars.empty())
				//throw string{"coerce: wanted string but has nothing"};
			string res = util::join(vars, " ");
			vars.clear();
			return res;
		}
		template<> Word coerce(vector<Variable> &vars) {
			if(vars.empty())
				throw string{"coerce: wanted word but has nothing"};
			string res = vars.front().toString();
			vars.erase(vars.begin());
			return Word{res.begin(), res.end()};
		}
		template<> long coerce(vector<Variable> &vars) {
			if(vars.empty())
				throw string{"coerce: wanted int but has nothing"};
			long var = vars.front().asInteger().value.l;
			vars.erase(vars.begin());
			return var;
		}
		template<> double coerce(vector<Variable> &vars) {
			if(vars.empty())
				throw string{"coerce: wanted int but has nothing"};
			double var = vars.front().asDouble().value.d;
			vars.erase(vars.begin());
			return var;
		}
		template<> vector<Variable> coerce(vector<Variable> &vars) {
			auto copy = vars;
			vars.clear();
			return copy;
		}
		template<> Variable coerce(vector<Variable> &vars) {
			if(vars.empty())
				throw string{"coerce: wanted Variable but has nothing"};
			Variable var = vars.front();
			vars.erase(vars.begin());
			return var;
		}
		template<> Variable makeVariable(Variable var) { return var; }
	}
}

Module findModule(string mname) {
	for(auto mod : modules::modules)
		if(mod.name == mname)
			return mod;
	throw (string)"module " + mname + " nonexistant";
}

bool modules::init(Bot *bot) {
	if(modules_inited)
		return true;

	cerr << "moudles::init: " << endl;
	defineModules();

	//cerr << "    dictionary size: " << global::dictionary.size() << endl;

	setupFunctions(bot);
	return true;
}

#include "modules.cpp.gen"

